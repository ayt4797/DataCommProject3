#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../include/utility.h"
#include <dirent.h>


struct sockaddr_in server_addr, new_addr;


int handleCommands(char* buffer,int sock);
void sendfilesize(FILE* fd,int sock);
void exit_session(int sock);
int main(){

	char *ip="0.0.0.0";
	int port =8884;
	int binding;
	int listening;
    char *ret;
	int sockfd, new_sock;

	socklen_t addr_size;
	char buffer[SIZE];

	sockfd =socket(AF_INET,SOCK_STREAM,0);
	
	if(sockfd<0){
		perror("TCP socket failed");
		return -99;
	}

	server_addr.sin_family=AF_INET;
	server_addr.sin_port=port;
	server_addr.sin_addr.s_addr=inet_addr(ip);

	binding=bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

	if(bind<0){
		perror("bind fialed");
		return -99;
	}
	printf("past bind");

	listening = listen(sockfd,10);

	if(listening==0){
		printf("listening");
	}
	else{
		perror("error on listening");
		return -99;
	}

	addr_size = sizeof(new_addr);

	new_sock = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size);
	printf("past socket creation");
	fflush(stdout);
	// send(new_sock,"Hello")
	// close(sockfd)
	int returnstatus=0;
	while(1){
		recv(new_sock,buffer,SIZE,0);
		if(handleCommands(buffer,new_sock)<0){
			perror("error on handle command\n");
				close(new_sock);
				return 0;
		}
		printf("finished a loop");
		fflush(stdin);
	}
	close(new_sock);
	return 0;
}

int handleCommands(char* buffer,int sock){
	char* option = malloc(strlen(buffer)); 
	int returnstatus=0;
	if(strcmp(buffer,EXIT_SESSION)==0){
		exit_session(sock);
	}
	char* ret =strstr(buffer,"tput");
	if(ret){
		printf("handling tput\n");
		fflush(stdin);
		getOptionFromBuffer(buffer,option);
		unsigned long size = seperateSizeFromOption(option);
		returnstatus=write_file_here(sock,GetFileFromBuffer(buffer,"w+"),size);
		if(returnstatus<0){
			send_completion_ack(sock,"-99");
			perror("could not write file");
			returnstatus =-99;
		}
		else{
			send_completion_ack(sock,"99");
		}
		bzero(ret,SIZEOFCOMMAND);
	}
	ret =strstr(buffer,"tget");
	if(ret){
		printf("\nhandling tget\n");
		fflush(stdout);
		FILE* fd =GetFileFromBuffer(buffer, "r");
		if(fd==NULL){
			perror("FILE DOES NOT EXIST");
			fflush(stdout);

			send_completion_ack(sock,"-99");
			returnstatus= 0;
			
		}
		else{
			printf("FILE DOES EXIST");
			fflush(stdin);
			send_completion_ack(sock,"99");
			sendfilesize(fd,sock);
			send_file(fd,sock);
			returnstatus =0;
		}
		bzero(ret,SIZEOFCOMMAND);

	}
	ret =strstr(buffer,"TLIST");
	if(ret){
		char directory[MAX_NUM_BYTES_FROM_CWD];
		char* message;
		if (getcwd(directory, sizeof(directory)) != NULL) {
			printf("Current working dir: %s\n", directory);
			DIR *d;
			struct dirent *dir;
			d = opendir(".");
			message = malloc(strlen(directory));
			strcpy(message, directory);

			if (d)
			{
				while ((dir = readdir(d)) != NULL)
				{
					strcat(message,dir->d_name);
					strcat(message,"\n\0"); //right here

					printf("%s\n", dir->d_name);
					fflush(stdout);
				}
				closedir(d);
			}
			//condense all the file names together into a single string to send
			send(sock,message,strlen(message)+1,0);
			free(message);
		} else {
			perror("getcwd() error");
			returnstatus= -99;
		}
		bzero(ret,SIZEOFCOMMAND);

	}
	ret =strstr(buffer,"TCWD");
	if(ret){
		getOptionFromBuffer(buffer,option);
		fflush(stdout);
		if (chdir(option) == 0) {
			printf("dir: %s exists\n", option);
			send_completion_ack(sock,"99");
			returnstatus= 0;
		} else {
			send_completion_ack(sock,"-99");
			printf("dir doesn't exist\n");
			returnstatus= 0;
		}
		bzero(ret,SIZEOFCOMMAND);

	}

	printf("exiting handler");
	fflush(stdin);
	free(option);

	return returnstatus;
}

void sendfilesize(FILE* fd,int sock){
	unsigned long  size = getfilesize(fd);
	printf("sending file of size: %lu\n",size);
	send(sock,&size,sizeof(unsigned long),0);
}

void exit_session(int sock){
	close(sock);
	exit(1);
}