// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #define SIZE 1024


// struct sockaddr_in server_addr, new_addr;

// void send_file(FILE *fp, int sockfd);

// int main(){

// 	char *ip="0.0.0.0";
// 	int port =8888;
// 	int connecting;
	
// 	int sockfd;
// 	FILE *fd;

// 	char* filename="tst.txt";

// 	sockfd =socket(AF_INET,SOCK_STREAM,0);
	
// 	if(sockfd<0){
// 		perror("TCP socket failed");
// 		return -99;
// 	}

// 	server_addr.sin_family=AF_INET;
// 	server_addr.sin_port=port;
// 	server_addr.sin_addr.s_addr=inet_addr(ip);
// 	connecting = connect(sockfd,(struct sockaddr*)&server_addr, sizeof(server_addr));

// 	if(connecting<0){
// 		perror("error in connecting");
// 		return -99;
// 	}
// 	printf("Connected to server");

// 	fd = fopen(filename,"r");

// 	if(fd==0){
// 		perror("can't find file");
// 		return -99;
// 	}

// 	send_file(fd,sockfd);
	
// 	printf("file sent");
// 	close(sockfd);
// 	return 0;
// }

// void send_file(FILE *fd, int sockfd){
// 	char data[SIZE]={0};
// 	int sendfile=0;
// 	while (fgets(data, SIZE,fd)!=NULL)	{
// 		sendfile= send(sockfd,data,sizeof(data),0);
// 		if(sendfile==-1){
// 			perror("could not send");
// 			exit(1);
// 		}
// 		bzero(data,SIZE);
	
// 	}
	

// }
