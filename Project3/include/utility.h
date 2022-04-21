#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define SIZE 1024 //max size of fgets not tcp
#define MAX_NUM_BYTES_FROM_CWD 1000
#define SIZEOFCOMMAND 4
#define EXIT_SESSION "EXIT SESSION"
void getOptionFromBuffer(char* buffer, char* filenameBuffer); //this parses is the past message for the option
FILE* GetFileFromBuffer(char* buffer, char* readWrite); //this calls get option from buffer & returns a file
unsigned long int seperateSizeFromOption(char* option);
FILE * getFile(char* filename);
unsigned long int getfilesize(FILE *fd);
void send_completion_ack(int sock,char* isSuccess);
void send_file(FILE *fd, int sockfd);
int write_file_here(int sockfd,FILE *fd,unsigned long expectedSize);
short recvAck(int sockfd);