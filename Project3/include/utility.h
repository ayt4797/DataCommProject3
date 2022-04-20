#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define SIZE 1024 //max size of fgets not tcp
#define MAX_NUM_BYTES_FROM_CWD 1000
FILE* getFileFromBuffer(char* buffer,char* readWrite,char* ret);
FILE * getFile(char* filename);
unsigned long int getfilesize(FILE *fd);
void send_completion_ack(int sock,char* isSuccess);
void send_file(FILE *fd, int sockfd);
int write_file_here(int sockfd,FILE *fd,unsigned long expectedSize);
short recvAck(int sockfd);