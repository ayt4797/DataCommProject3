#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "../include/utility.h"


struct sockaddr_in server_addr, new_addr;


int handleCommands(char* buffer,int sock);
void sendfilesize(FILE* fd,int sock);

int main(){

	char *ip="0.0.0.0";
	int port =8888;
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
	// send(new_sock,"Hello")
	// close(sockfd)
	int returnstatus=0;
	while(1){
		recv(new_sock,buffer,SIZE,0);
		if(handleCommands(buffer,new_sock)<0){
			perror("error on handle command\n");
			continue;
		}
		printf("finished a loop");
		fflush(stdin);
	}

	return 0;
}

int handleCommands(char* buffer,int sock){
	char* ret =strstr(buffer,"tput");
	int returnstatus=0;
	if(ret){
		printf("handling tput\n");
		fflush(stdin);

		unsigned long int size;
		printf("ret + 5: %s",ret+5);
		sscanf(ret+5, "%lu", &size);
		printf("size : %lu \n",size);
		fflush(stdin);
				
		returnstatus=write_file_here(sock,getFileFromBuffer(buffer,"w+",ret),size);
		if(returnstatus<0){
			send_completion_ack(sock,"-99");
			perror("could not write file");
			exit(1);
		}
		else{
			send_completion_ack(sock,"99");
		}
	}
	ret =strstr(buffer,"tget");
	if(ret){
		printf("handling tget\n");
		fflush(stdin);
		FILE* fd =getFileFromBuffer(buffer, "r",ret);
		if(fd==NULL){
			perror("FILE DOES NOT EXIST");
			fflush(stdin);

			send_completion_ack(sock,"-99");
			return 0;
			
		}
		else{
			printf("FILE DOES EXIST");
			fflush(stdin);
			send_completion_ack(sock,"99");
			sendfilesize(fd,sock);
			send_file(fd,sock);
			returnstatus =0;
		}
	}
	printf("exiting handler");
	fflush(stdin);


	return returnstatus;
}

void sendfilesize(FILE* fd,int sock){
	unsigned long  size = getfilesize(fd);
	printf("sending file of size: %lu\n",size);
	send(sock,&size,sizeof(unsigned long),0);
}

