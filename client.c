#define h_addr h_addr_list[0] /* for backward compatibility */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <malloc.h>
#include <pthread.h>

#define BUFFER_SIZE 512
#define CHUNK_SIZE 16

typedef struct {
	int serverfd;				/* Server file discriptor */
} server_t;

char *ring_buff = NULL;
int ring_head = 0, ring_tail = 0;
int ring_buff_initiated = 0;

void *session(void*);
void *read_session(void*);
int ring_buff_init(int buff_len);
int ring_add(const char *buff, int buff_len);
int ring_get(char *buff_out, int len);
int ring_remove(int len);
int ring_used_len();
int ring_find_magic(const char *magic, int len);
void ring_buff_free(void); /* 'void' is needed for declairation */
char *substring(char *string, int position, int length);


int main(int argc, char *argv[])
{
	int sockfd, port;
        struct sockaddr_in serv_addr;
        struct hostent *server;
	char buff[BUFFER_SIZE];
	pthread_t thread; /* Thread for output */
	server_t* serverfd;

	int ring_buff_initiated = 0;
	int file_ended = 0;
	char *file_name_l = NULL, *received_msg_l = NULL;
	int file_name_len = 0, received_msg_len = 0;
	char *file_name = NULL, *received_msg = NULL;
	char *ring_chunk = NULL;
/*	FILE* fp = NULL;
*/
	if (argc != 3) {
        	fprintf(stderr,"usage %s hostname port\n", argv[0]);
        	return 0;
        }
	
	port = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("ERROR opening socket.");
		return 1;
	}
	server = gethostbyname(argv[1]);
	if (server == NULL)
	{
		fprintf(stderr,"ERROR, no such host: %s\n", argv[1]);
		return 1;
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(port);
	if(connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("ERROR connecting");
		return 1;
	}
	serverfd = malloc(sizeof(server_t));
	serverfd->serverfd = sockfd;
	if(pthread_create(&thread, NULL, &session, (void*) serverfd))
	/*if(pthread_create(&thread, NULL, &session, (void*) serverfd) && pthread_create(&thread, NULL, &read_session, (void*) serverfd) )*/
	{
		close(sockfd);
	}
	bzero(buff, BUFFER_SIZE);
	while((read(sockfd, buff, BUFFER_SIZE))>0)
	{
/*		if ( strstr(buff, "\\*") != NULL )
		if ( !strcmp(buff, "[EncodedMsgOfSentFile]", 23)  )
		{
			printf("Will write msg to a file.[%s] \n", buff);
			ring_buff_init(strlen(buff));

			if (fp == NULL)
			{
				fp = fopen("ReceivedFile.txt", "w");
			}

			if (!strncmp(buff, "\\End_of_file_sent", 17))
			{
				fclose(fp);
				fp = NULL;
			}
			else
			{
				if (strstr(buff, ))
				fwrite(buff, strlen(buff), 1, fp);

				ring_buff_add(buff, strlen(buff));
				printf("One msg writen:%s. \n", buff);
			}
		}
*/

		if (strstr(buff, "\\*") != NULL)
		{

			if (!strncmp(buff, "\\*End_of_file_sent", 18)) file_ended = 1;
			printf("buff is file: %s \n", buff);
			printf("buff size: %d \n", sizeof(buff));
			printf("buff len: %d \n", strlen(buff));
/*			if (ring_buff_initiated == 0)
			{
/ *				if (ring_buff_init(strlen(buff)) == -1) return -1;
* /
				if (ring_buff_init(strlen(buff)) == -1)
				{
					printf("str len buff: %d\n", strlen(buff));
					printf("buff is file 22: %s \n", buff);
					return -1;
				}
				else ring_buff_initiated = 1;
			}
*/
			printf("buff received: %s \n", buff);
			printf("buff strlen: %d \n", strlen(buff));

			printf("ring_buff added: %s \n", ring_buff);

			file_name_l = (char *)malloc(3);
			received_msg_l = (char *)malloc(5);
			ring_chunk = (char *)malloc(2*CHUNK_SIZE); /* to free() */

			memcpy(file_name_l, buff + 2, 2);
			memcpy(received_msg_l,buff + 4, 4);
			file_name_len = atoi(file_name_l);
			received_msg_len = atoi(received_msg_l);
			printf("file_name_l: %s \n", file_name_l);
			printf("received_msg_l: %s \n", received_msg_l);
			printf("file_name_len: %d \n", file_name_len);
			printf("received_msg_len: %d \n", received_msg_len);

			file_name = (char *)malloc(file_name_len); /* to free() */
			received_msg = (char *)malloc(received_msg_len); /* to free() */
			strncpy(file_name, buff + 9, file_name_len);
/*			file_name = substring(buff, 9, file_name_len);
*/
			printf("file_name: %s \n", file_name);

                        ring_buff_init(BUFFER_SIZE);
                        printf("ring_buff_size: %d\n", sizeof(ring_buff));
                        printf("ring_buff_ len : %d\n", strlen(ring_buff));
                        printf("ring_buff_initiated: %d\n", ring_buff_initiated);
			if (ring_add(buff, strlen(buff)) == -1) return -1;

			printf("ring_used_len() = %d \n", ring_used_len());

			while (ring_used_len() > 0)
			{
				if(ring_used_len() >= 2 + 2 + 4 + file_name_len + received_msg_len)
				{
					ring_get(received_msg, 2 + 2 + 4 + file_name_len + received_msg_len);
					ring_remove(2 + 2 + 4 + file_name_len + received_msg_len);
					ring_chunk = substring(received_msg, 2 + 2 + 4 + file_name_len, received_msg_len);
					printf("received_msg: %s \n", received_msg);
					printf("ring_chunk: %s \n", ring_chunk);
					continue;
/*					received_msg = substring(buff, 8 + file_name_len, strlen(buff) - (8 + file_name_len));
*/				}
				if(file_ended == 1)
				{
					ring_get(received_msg, ring_used_len());
					ring_buff_free();
					ring_chunk = substring(received_msg, 2 + 2 + 4 + file_name_len, ring_used_len() - (2 + 2 + 4 + file_name_len));
					printf("end ring_chunk: %s \n", ring_chunk);
					free(file_name);
					free(received_msg);
					free(ring_chunk);
				}
			}
			
		}
		else
		{
			printf("Just a message: \n");
			printf("%s\n", buff);
		}
		bzero(buff, BUFFER_SIZE);
	}
	close(sockfd);
	return 0;
}

void* session(void *args)
{
	char buff[BUFFER_SIZE];
	server_t* tmp;
	int sockfd;
	tmp = (server_t*) args;
	sockfd = tmp->serverfd;
	bzero(buff, BUFFER_SIZE);
	while(strcmp(buff,"\\quit"))
	{
		fgets(buff, sizeof(buff), stdin);
		write(sockfd, buff, BUFFER_SIZE);
		bzero(buff, BUFFER_SIZE);
	}
	pthread_detach(pthread_self());
	return NULL;
}


/*client's read_session*/
void* read_session(void *args)
{
        char buff[BUFFER_SIZE];
        server_t* tmp;
        int sockfd;
        tmp = (server_t*) args;
        sockfd = tmp->serverfd;
        bzero(buff, BUFFER_SIZE);

printf("read_session is called");
	while((read(sockfd, buff, BUFFER_SIZE))>0)
	{
		printf("%s", buff);
		bzero(buff, BUFFER_SIZE);
	}
	close(sockfd);

        pthread_detach(pthread_self());
        return NULL;
}

int ring_buff_init(int buff_len)
{
printf("In ring_buff_init, buff_len is: %d \n", buff_len);
printf("In ring_buff_init, ring_buff_size is: %d \n", sizeof(ring_buff));
	if (ring_buff != NULL) return -1;
	if ((ring_buff = (char *)malloc(buff_len + 4)) == NULL) return -1;
printf("In ring_buff_init, ring_buff_size is: %d \n", sizeof(ring_buff));
printf("In ring_buff_init, ring_buff_len is: %d \n", strlen(ring_buff));
	ring_head = 0;
	ring_tail = 0;
	return 0;
}

void ring_buff_free()
{
	if (ring_buff == NULL) return;
	free(ring_buff);
	/* the function always 'return', so no need 'return'  */
}


int ring_add(const char *buff, int buff_len)
{
printf("buff_len is: %d \n", buff_len);
printf("ring_buff_size is: %d  \n", sizeof(ring_buff));
printf("buff was: %s  \n", buff);
printf("ring_buff was: %s  \n", ring_buff);
	if (ring_used_len()>0) /* sizeof(ring_buff) was 4 initially.  */
	{
		if (buff_len > sizeof(ring_buff) - ring_used_len()) return -1;  /* can replace below? */
	}
/*
	if (ring_head > ring_tail)
	{
		if (buff_len > sizeof(ring_buff) - (ring_head - ring_tail))
			 return -1;
	}
	else
                if (buff_len > (ring_tail - ring_head))
                         return -1;
*/
	if (buff_len > (sizeof(ring_buff) - ring_head))
	{
		int cut_len = (sizeof(ring_buff) - ring_head);
		memcpy(ring_buff + ring_head, buff, cut_len);
		memcpy(ring_buff, buff + cut_len, buff_len - cut_len);
		ring_head = buff_len - cut_len;
	}
	else
	{

printf("ring_head: %d  \n", ring_head);
printf("ring_tail: %d  \n", ring_tail);
		memcpy(ring_buff + ring_head, buff, buff_len);
		ring_head += buff_len;
	}

	if (ring_head == sizeof(ring_buff)) ring_head = 0;
printf("ring_add() is over. \n");
printf("ring_buff is: %s  \n", ring_buff);

	return 0;
}


int ring_get(char *ring_chunk, int len)  /* need to modify buff_out, so no 'const' . */
{
	if (ring_used_len() < len) return -1;
	if ((ring_tail + len) <= sizeof(ring_buff)) 
	{
		memcpy(ring_chunk, ring_buff + ring_tail, len);  /*careful about '<=' */
	}
	else
	{
		memcpy(ring_chunk, ring_buff + ring_tail, sizeof(ring_buff) - ring_tail); /*doing with data, so can't use str functions  */
		memcpy(ring_chunk + (sizeof(ring_buff) - ring_tail), ring_buff, len - (sizeof(ring_buff) -ring_tail));
	}
	return 0;
}

int ring_remove(int len)
{
	if (ring_used_len() < len) return -1;
	if ((ring_tail + len) <= sizeof(ring_buff))
	{
		ring_tail = ring_tail + len;
		if (ring_tail == sizeof(ring_buff)) ring_tail = 0;
	}
	else
	{
		ring_tail = len - (sizeof(ring_buff) - ring_tail);
	}
	return 0;
}


int ring_used_len()
{
	if (ring_head >= ring_tail) return ring_head - ring_tail;
	return (sizeof(ring_buff) - ring_tail) + ring_head;
}


int ring_find_magic(const char *magic, int len)
{
	int i;

	if (ring_used_len() < len) return -1;

	for (i = 0; i < ring_used_len(); i++)
	{
		if (*(ring_buff + ring_tail + i) == '\\')
 		{
			if ((ring_tail + i) == sizeof(ring_buff))  /* Is other way better?  */
			{
				if (*(ring_buff) == '*') return 0; 
			}
			else
			{
				if (*(ring_buff + ring_tail + i + 1) == '*') return 0;
			}
		}
	}

	return -1;
}


/*C substring function: It returns a pointer to the substring */

char *substring(char *string, int position, int length)
{
   char *pointer;
   int c;

   pointer = malloc(length+1);

   if (pointer == NULL)
   {
      printf("Unable to allocate memory.\n");
      exit(1);
   }

   for (c = 0 ; c < length ; c++)
   {
      *(pointer+c) = *(string+position-1);
      string++;
   }

   *(pointer+c) = '\0';

   return pointer;
}








