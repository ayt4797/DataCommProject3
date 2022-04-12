#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define SIZE 1024


struct sockaddr_in server_addr, new_addr;

FILE* getfile(char* buffer,char* readWrite,char* ret);

int handleCommands(char* buffer,int sock);

void send_completion_ack(int sock,unsigned short isSuccess);

void send_file(FILE *fd, int sockfd);

int write_file_here(int sockfd,FILE *fd,unsigned long expectedSize);

unsigned long int getfilesize(FILE *fd);

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
FILE* getfile(char* buffer,char* readWrite,char* ret){
	char* filename= malloc(ret-buffer-1);

	memcpy(filename,buffer,(ret-buffer-1));
	printf("filename %s\n", filename);
	printf("buffer %s\n", buffer);
	fflush(stdin);

	FILE* fd = fopen(filename,readWrite);
	if(fd<0){
		perror("could not open file");
	}
	free(filename);
	printf("END OF GET FILE \n\n\n");
	return fd;
	
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
				
		returnstatus=write_file_here(sock,getfile(buffer,"w",ret),size);
		if(returnstatus<0){
			send_completion_ack(sock,-99);
			perror("could not write file");
			exit(1);
		}
		else{
			send_completion_ack(sock,1);
		}
	}
	ret =strstr(buffer,"tget");
	if(ret){
		printf("handling tget\n");
		fflush(stdin);
		FILE* fd =getfile(buffer, "r",ret);
		if(fd==0){
			printf("FILE DOES NOT EXIST");
			fflush(stdin);

			send_completion_ack(sock,-1);
			return 0;
			
		}
		else{
			printf("FILE DOES  EXIST");
			fflush(stdin);
			send_completion_ack(sock,1);
			send_file(fd,sock);
			returnstatus =0;
		}
	}
	printf("exiting handler");
	fflush(stdin);


	return returnstatus;
}

void sendfilesize(){
	
}

unsigned long int getfilesize(FILE *fd){
    unsigned long int prev=ftell(fd); 
    fseek(fd, 0L, SEEK_END);
    unsigned long int sz=ftell(fd);
    fseek(fd,prev,SEEK_SET); //go back to where we were
    return sz;
}


void send_completion_ack(int sock,unsigned short isSuccess){
	int sendfile= send(sock,&isSuccess,2,0);
	if(sendfile<=0){
		perror("error on send ack");
		fflush(stdin);
		exit(1);
	}
}


void send_file(FILE *fd, int sockfd){
	char data[SIZE]={0};
	int sendfile=0;
	while (fgets(data, SIZE,fd)!=NULL)	{
		sendfile= send(sockfd,data,sizeof(data),0);
		if(sendfile==-1){
			perror("could not send");
			exit(1);
		}
		bzero(data,SIZE);
	
	}
}
int write_file_here(int sockfd,FILE *fd,unsigned long expectedSize){
	int recieve;
	char buffer[SIZE];
	unsigned long count =0;
	while(count<expectedSize){
		recieve= recv(sockfd,buffer,SIZE,0);
		count+=recieve;
		fflush(stdin);
		if(recieve==0){//should be less than 0 for err
			break;
		}
		if(recieve <=-1){
			perror("err on recieve");
			return -99;
		}
		fprintf(fd,"%s",buffer);
		fflush(stdin);
		bzero(buffer,SIZE);
	}
	printf("out of write file here method\n");
	fflush(stdin);
	return 0;
}
