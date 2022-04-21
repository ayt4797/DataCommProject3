#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../include/utility.h"
#include <dirent.h>
#define MAXCLIENTS 200

struct sockaddr_in server_addr, new_addr;


int handleCommands(char* buffer,int sock);
void sendfilesize(FILE* fd,int sock);
void exit_session(int sock);

/**
 * @brief is the main
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[]) {
	
	char *ip="0.0.0.0";
	int port =DEFAULT_PORT;
	if(argc <= 2){
		printf("I need a ip, defaulting to localhost");
		fflush(stdout);
	}
	else{
		ip = argv[1];
	}

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
	int listeningOnAcceptId;
	while(1){
		new_sock = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size);
		if(new_sock<0){
			perror("will not accept!: ");
			}
		listeningOnAcceptId=fork();
		if(listeningOnAcceptId<0){
			perror("FORK FAILED!: ");
		}
		
		printf("\nacceptanace!");
		fflush(stdout);
		if(!listeningOnAcceptId){ //!0=1
			break;
		}
	
	}
	while(1){
		bzero(buffer,SIZE);
		int r =recv(new_sock,buffer,SIZE,0);
		if(r<0){
			perror("Err on recieve: ");
		}
		if(r>0&&handleCommands(buffer,new_sock)<0){
			perror("error on handle command\n");
			close(new_sock);
			return 0;
		}
		if(strcmp(buffer,EXIT_SESSION)==0){
			printf("EXITING");
			close(new_sock);
			break;
		}
		fflush(stdin);
	}
	close(new_sock);
	return 0;
}
/**
 * @brief this checks the buffer, and seperate it from the option, then calls function from utility.c 
 * to actually handle the command.
 * 
 * @param buffer 
 * @param sock 
 * @return int 
 */

int handleCommands(char* buffer,int sock){
	char* option = malloc(strlen(buffer)); 
	if(option==NULL){
		perror("MALLOC FAILED: ");
	}
	int returnstatus=0;
	printf("BUFFER %s", buffer);
	char* ret =strstr(buffer,"tput");
	if(ret){
		printf("handling tput\n");
		fflush(stdin);
		getOptionFromBuffer(buffer,option);
		unsigned long size = seperateSizeFromOption(option);
		returnstatus=write_file_here(sock,GetFileFromFilename(option,"w+"),size);
		if(returnstatus<0){
			perror("could not write file");
			returnstatus =-99;
		}
		bzero(ret,SIZEOFCOMMAND);
	}
	ret =strstr(buffer,"tget");
	if(ret){
		printf("\nhandling tget\n");
		fflush(stdout);
		getOptionFromBuffer(buffer,option);
		FILE* file = getFile(option);//purely for checking if the file exists
		if(file==NULL){
			send_completion_ack(sock,-99);
			returnstatus =0;
			}
		else{
			fclose(file);
			send_completion_ack(sock, 99);
			unsigned long size = getfilesize(getFile(option));
			int s =send(sock,&size,sizeof(unsigned long),0);
			if(s==-1){
				perror("error on sending: ");
			}
			short sf = send_file(option, sock,size);  
			if(sf<0)
				perror("error on sending file");
			bzero(ret,SIZEOFCOMMAND);
		}
	}
	ret =strstr(buffer,"TLIST");
	if(ret){
		printf("TLIST COMMAND CALLED");
		char directory[MAX_NUM_BYTES_FROM_CWD];
		char* message;
		if (getcwd(directory, sizeof(directory)) != NULL) {
			printf("Current working dir: %s\n", directory);
			DIR *d;
			struct dirent *dir;
			d = opendir(".");
			message = malloc(strlen(directory)+1);
			strcpy(message, directory);

			if (d)
			{
				while ((dir = readdir(d)) != NULL)
				{
					message = realloc(message,strlen(message)+strlen(dir->d_name)+2);
					if(message==NULL){
						perror("could not realloc");
					}
					strcat(message,dir->d_name);
					strcat(message,"\n"); //right here

					printf("%s\n", dir->d_name);
					fflush(stdout);
				}
				closedir(d);
			}
			//condense all the file names together into a single string to send
			unsigned long long int size= strlen(message)+1; //send the size of that message
			int s=send(sock,&size,sizeof(unsigned long long int),0); 
			if(s<0){
				perror("ERROR ON SENDING SIZE: ");
			}

			s=send(sock,message,strlen(message)+1,0);
			if(s<0){
				perror("ERROR ON SENDING TLIST: ");
			}
			free(message);
		} else {
			perror("getcwd() error");
			returnstatus= -99;
		}
		bzero(ret,5);

	}
	ret =strstr(buffer,"TCWD");
	if(ret){
		getOptionFromBuffer(buffer,option);
		printf("option: %s",option);
		fflush(stdout);
		if (chdir(option) == 0) {
			printf("dir: %s exists\n", option);
			send_completion_ack(sock,99);
			returnstatus= 0;
		} else {
			send_completion_ack(sock,-99);
			printf("dir doesn't exist\n");
			returnstatus= 0;
		}
		bzero(ret,SIZEOFCOMMAND);

	}
	free(option);

	return returnstatus;
}

/**
 * @brief sends the filesize  not in ascii format, but as bytes.
 * 
 * @param fd 
 * @param sock 
 */

void sendfilesize(FILE* fd,int sock){
	unsigned long  size = getfilesize(fd);
	printf("sending file of size: %lu\n",size);
	if(send(sock,&size,sizeof(unsigned long),0)<0){
		perror("Error on sending");
	}
}