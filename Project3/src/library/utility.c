#include "../../include/utility.h"

void getOptionFromBuffer(char* buffer, char* filenameBuffer){
	printf("buffer (pre uniforming) %s\n", buffer);
	strcpy(filenameBuffer,buffer+4);
	printf("File name :%s\n", filenameBuffer);
	fflush(stdout);
}

unsigned long int seperateSizeFromOption(char* option){
	unsigned long int size;
	char message[SIZE];
	sscanf(option, "%s %lu",message, &size);
	printf("message :%s\n",message);
	strcpy(option,message);
	printf("option :%s\n",option);
	printf("size : %lu \n",size);
	fflush(stdin);
	
	return size;
}

FILE* GetFileFromFilename(char* buffer, char* readWrite){
	char filename[SIZE];
	printf("opening file %s\n",filename);
	FILE* fd = fopen(filename,readWrite);
	if(fd<0){
		perror("could not open file");
	}
	printf("END OF GET FILE \n\n\n");
	return fd;
}

//returns a file descriptor for a file, returns NULL if the file doesn't exist
FILE * getFile(char* filename){
	FILE *file;
    
    if (file = fopen(filename, "r")) {
		printf("file exists");
    }
    else
    {
        printf("file %s doesn't exist",filename);
    }
	fflush(stdin);
	return file;
}
unsigned long int getfilesize(FILE *fd){
	rewind(fd);
    unsigned long int prev=ftell(fd); 
    fseek(fd, 0L, SEEK_END);
    unsigned long int sz=ftell(fd);
    fseek(fd,prev,SEEK_SET); //go back to where we were
	printf("SIZE: %lu",sz);
    return sz;
}

void send_completion_ack(int sock,char* isSuccess){
	printf("\nack value sending: %s\n",isSuccess);
	int sendfile= send(sock,isSuccess,SIZEOFACK,0);
	if(sendfile<=0){
		perror("error on send ack");
		fflush(stdin);
		exit(1);
	}
}
short recvAck(int sockfd){
	
	char buffer[SIZEOFACK];
	int r = recv(sockfd,&buffer,SIZEOFACK,0);
	if(r<0){
		perror("couldn't recv");
		return -99;
	}

	int ack=0;
	sscanf(buffer,"%d",&ack);
	printf("ACK VALUE: %s\n",buffer);
	printf("ACK VALUE: %d\n",ack);


	if(ack<0){
		return -99;
	}
	else{
		return 99;
	}
}

//a wrapper for the sendfile method
int send_file(char* file, int sockfd,unsigned long size){
	int f;
	short sf = sendfile(sockfd,f=open(file,'r',O_CREAT),NULL,size);
	close(f);
	if(sf<0)
		return -99;
	return sf;
}
int write_file_here(int sockfd,FILE *fd,unsigned long expectedSize){
	int recieve=0; //can't be unsigned b/c can be negative
	char buffer[SIZE];
	unsigned long count =0;
	printf("expected size: %lu\n",expectedSize );
	fflush(stdin);

	while(count<expectedSize){
		recieve= recv(sockfd,buffer,SIZE,0);
		count+=recieve;
		fflush(stdin);
		printf("Recieved bits: %d\n",recieve );
		printf("counted bits: %lu\n",count );
		fflush(stdin);
		if(recieve==0){//should be less than 0 for err
			break;
		}
		if(recieve <=-1){
			perror("err on recieve");
			return -99;
		}
		printf("%s\n",buffer);
		int printReturn = fputs(buffer,fd);
		fflush(stdout);

		
		if(printReturn<0){
			perror("error on writing file");
			return -99;
		}
		fflush(stdin);
		bzero(buffer,SIZE);
	}
	fclose(fd);
	printf("out of write file here method\n");
	fflush(stdin);
	return 0;
}

