#include "../../include/utility.h"

/**
 * @brief Get the Option From Buffer object
 * simply adds the buffer +4 then copies that to an outbuffer
 * @param buffer 
 * @param filenameBuffer 
 */
void getOptionFromBuffer(char* buffer, char* filenameBuffer){
	printf("buffer: %s\n", buffer);
	strcpy(filenameBuffer,buffer+4);
	printf("File name :%s\n", filenameBuffer);
	fflush(stdout);
}
/**
 * @brief scans the option seperated from the buffer
 * seperates the buffer from the size that was attatched to the original message
 * using sscanf
 * @param option 
 * @return unsigned long int 
 */
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
/**
 * @brief Get the File From Filename object
 * 
 * @param buffer 
 * @param readWrite 
 * @return FILE* 
 */
FILE* GetFileFromFilename(char* buffer, char* readWrite){
	char filename[SIZE];
	strcpy(filename,buffer);
	FILE* fd = fopen(filename,readWrite);
	if(fd<0){
		perror("could not open file");
	}
	printf("END OF GET FILE \n\n\n");
	return fd;
}
/**
 * @brief Get the File object returns a file descriptor for a file, returns NULL if the file doesn't exist
 * 
 * @param filename 
 * @return FILE* 
 */
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
/**
 * @brief goes from the top of the file to get the size of the file from the fd
 * 
 * @param fd 
 * @return unsigned long int 
 */
unsigned long int getfilesize(FILE *fd){
	rewind(fd);
    unsigned long int prev=ftell(fd); 
    fseek(fd, 0L, SEEK_END);
    unsigned long int sz=ftell(fd);
    fseek(fd,prev,SEEK_SET); //go back to where we were
	printf("SIZE: %lu",sz);
    return sz;
}

/**
 * @brief sends 4 characters to inform the user if the prevoius action was a sucess or a failure
 * 
 * @param sock 
 * @param isSuccess 
 */
void send_completion_ack(int sock,short isSuccess){
	printf("\nack value sending: %d\n",isSuccess); //strlen(isSuccess)+1
	int sendfile= send(sock,&isSuccess,SIZEOFACK,0);
	if(sendfile<=0){
		perror("error on send ack");
		fflush(stdout);
		// exit(1);
	}
}
/**
 * @brief recieves the send_completion_ack function
 * 
 * @param sockfd 
 * @return short 
 */
short recvAck(int sockfd){
	
	short buffer;
	int r = recv(sockfd,&buffer,SIZEOFACK,0);
	if(r<0){
		perror("couldn't recv");
		return -99;
	}

	printf("ACK VALUE: %d\n",buffer);


	if(buffer<0){
		return -99;
	}
	else{
		return 99;
	}
}

/**
 * @brief a wrapper for the sendfile method
 * 
 * @param file 
 * @param sockfd 
 * @param size 
 * @return int 
 */
int send_file(char* file, int sockfd,unsigned long size){
	int f;
	short sf = sendfile(sockfd,f=open(file,'r',O_CREAT),NULL,size);
	close(f);
	if(sf<0){
		perror("error on sending the file: ");
		return -99;
		}
	return sf;
}
/**
 * @brief waits on the port using recv and reads SIZE amount from the buffer
 * and then writes them to a file.
 * @param sockfd 
 * @param fd 
 * @param expectedSize 
 * @return int 
 */
int write_file_here(int sockfd,FILE *fd,unsigned long expectedSize){
	int recieve=0; //can't be unsigned b/c can be negative
	char buffer[SIZE];
	unsigned long count =0;
	printf("expected size: %lu\n",expectedSize );
	fflush(stdin);

	while(count<expectedSize){
		bzero(buffer,SIZE);
		recieve= recv(sockfd,buffer,SIZE,0);
		count+=recieve;
		printf("Recieved bits: %d\n",recieve );
		printf("counted bits: %lu\n",count );
		fflush(stdout);
		if(recieve==0){//should be less than 0 for err
			break;
		}
		if(recieve <=-1){
			perror("err on recieve");
			return -99;
		}
		// printf("%s\n",buffer);
		int printReturn = fputs(buffer,fd);
		fflush(stdout);

		
		if(printReturn<0){
			perror("error on writing file");
			return -99;
		}
		fflush(stdout);
	}
	fclose(fd);
	printf("out of write file here method\n");
	fflush(stdout);
	return 0;
}

