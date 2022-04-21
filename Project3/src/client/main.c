
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>

#include "../../include/processing.h"
#include "../../include/utility.h"

/*
 * Add any socket variables and other variables you deem necessary
 */
fd_set rfds;
int p[2];
int s;
#define TIMER 1
/*
 * Fill in code for the Login process here. This function is called by the
 * lexer library wheen the user insantiaties the TCONNECT command. Be sure
 * to return a success status of 0 or failure status of -1. This informs
 * the lexer code of the authentication status. Note that this only accounts
 * for the authentication of the client side. Be sure you support an
 * authentication with the server side as well.
 * 
 * @param uint32_t  IPv4 Address of the server as a 32-bit number
 * @param char *    NULL-terminated Usersname char array
 * @param char *    NULL-terminated Password char array
 * 
 * @returns int     Success status: 0 success, -1 failure
 */
int sockfd;
int create_connection();

int sendTGETMessage(int sockfd,char* filename);
int sendTPUTMessage(int sockfd, char* filename);
void assembleMessage(char* command, char* options, char* result);
void assembleTputMessage(char* command, char* option, unsigned long size, char* result );
int ProcessLogin( uint32_t sipaddr, char *username, char *password ) {
  	printf("Library called the login function <%s> <%s>\n", username, password);
  // Place your connection and authentication process code here
	sockfd= create_connection();//OPEN connection
	if(sockfd<0){
		perror("failure on conenction");
		return -99;
	}
	if (pipe(p) < 0)
	{
		printf("piping failed");
		exit(0);
	}

	struct timeval timev;
	timev.tv_sec = TIMER;
	timev.tv_usec = 0;

	FD_ZERO(&rfds);
	FD_SET(sockfd, &rfds);
	FD_SET(p[0], &rfds);
	FD_SET(0, &rfds); //stdin

	s = select(1024, &rfds, NULL, NULL, &timev);
	if (s == -1)
		{
			perror("select failed!");
			exit(1);
		}
  Authenticated();
  return 0;
}

/*
 * This function is called by the lexer code when the user submits the exit 
 * command after the client is already logged in and authenticated with the 
 * server.  This function should close the connection with the server and 
 * then call the Deauthenticate() function.
 * 
 * @return int      Success status: 0 success, -1 failure
 */
int ProcessLogout() {
  // Place your disconnection and memory deallocation code here
  Deauthenticate();

}

/*
 * Fill in code for the TPUT command. This function accepts a NULL-terminated
 * character array containing the filename of the local filesystem file to 
 * push to the server. You must open and read the local file, and then send the 
 * file to the serveer along with the filename of the file. Return the status 
 * of the operation as an integer value (using POSIX standards).
 * 
 * @param char*     Filename of the local file to send
 * 
 * @returns int     Status of the operation 0 for success, -1 for failure
 */
int ProcessPutFile( char *filename ) {
  printf("Library called the TPUT function with file: <%s>\n", filename );
  // Place your File Upload code here

	int status =sendTPUTMessage(sockfd,filename);
	if(status<0){
		printf("continuing\n");
		fflush(stdout);
		return status;
	}
	printf("waiting on ack");
	short completion_value =recvAck(sockfd);
	if(completion_value <0){
		printf("An error occured sending the file");
		status =-99;
	}
  	return status;
}

/*
 * Fill in code for the TGET command. This function accepts a NULL-terminated
 * character array containing the filename of the server filesystem file to 
 * pull from the server. You must send the command and filename to the server.
 * The server should report a success/failure status to the clieent, and if 
 * success, it should then transmit the entire file.  This funciton must then
 * store the data using the same filename on the local filesystem.
 * 
 * @param char*     Filename of the remote file to receive
 * 
 * @returns int     Status of the operation 0 for success, -1 for failure
 */
int ProcessGetFile( char *filename ) {
  printf("Library called the TGET function with file: <%s>\n", filename );
  // Place your File Download code here
	sendTGETMessage(sockfd,filename);
	short completion_value=0;
	// if(s>0){
		// if(FD_ISSET(sockfd,&rfds)){
		completion_value =recvAck(sockfd);
			// }
	// }
	short status=0;
	if(completion_value <0){
		printf("The server does not have that file");
		status =-99;
	}
	else{
		unsigned long size=0;
		if(recv(sockfd,&size, sizeof(unsigned long),0)<0){

			perror("err on recv");
			status = -99;
		}
		FILE* fd =fopen(filename, "w+");
		printf("SIZE: %lu",size);
		fflush(stdout);
		write_file_here(sockfd,fd,size);
	}
  return status;
}

/*
 * This function should send a command to the server to get a listing of the 
 * server's current working directory, for the thread or context serving the 
 * client instance. This implies that the server should be able to have different
 * working directories active for each authenticated client. The server should 
 * return success or failure, and if success, it should alos provide the 
 * listing of the server's currwnt directory.
 * 
 * @returns int     Status of the operation 0 for success, -1 for failure
 */
int ProcessListDir() {
  	printf("Library called the TLIST function\n");
  // Place your Directory Listing code here
	//	sendfile= send(sockfd,data,sizeof(data),0);
  //

	send(sockfd,"TLIST",6,0);
	char dir[MAX_NUM_BYTES_FROM_CWD];
	if(recv(sockfd,dir, MAX_NUM_BYTES_FROM_CWD,0)<0){ //too many bytes.
		perror("error on recv");

		}
	printf("\nDIRECTORY : %s\n",dir);

  return 0;
}

/*
 * This function is used to change the working directory of the server for the 
 * active client only. Changing the server's working directory from one instance
 * of the client should have no affect on another active instance of the same 
 * client. The server should report success or failure.
 * 
 * @returns int     Status of the operation 0 for success, -1 for failure
 */
int ProcessChangeDir( char *dirname ) {
	printf("Library called the TCWD function: <%s>\n", dirname);
	fflush(stdout);
	char message[SIZE];
	assembleMessage("TCWD",dirname,message);
	send(sockfd,message,strlen(message)+1,0);
	if(recvAck(sockfd)<0){
		printf("that file doesn't exist");
		return -99;
	}
	else{
		printf("Succesfully changed directory");
	}
  return 0;
}

/*
 * This function is used to perform an emergency shutdown. If the STDIN stream
 * returns an EOF, then the client must close the active connection and clean
 * up any dyanmic memory or files opened. When you return from this function
 * the ProcessCommand loop will break out and return to the main function to 
 * complete the app processing.
 *
 */
void EmergencyShutdown() {
	send(sockfd,EXIT_SESSION,13,0);
	close(sockfd);
}


int sendTPUTMessage(int sockfd, char* filename){
	unsigned long int size =0;
	FILE* fd =getFile(filename);
	if(fd == NULL){
		return -99;
	}
	size = getfilesize(fd);
	fclose(fd);
	// printf("file size: %lu", size);
	char message[SIZE];
	assembleTputMessage("tput",filename,size,message);

	int sendcommand= send(sockfd,message,strlen(message)+1,0);
	if(sendcommand<0){
		perror("error on sending tput command");
	}
	printf("\nSENDING TPUT COMMAND!\n");
	fflush(stdout);

	sendcommand=send_file(filename,sockfd,size); //there is error checking in the send_file method
	if(sendcommand<0){
		perror("error on sending tput command");
	}
	return sendcommand;
}

int sendTGETMessage(int sockfd,char* filename){
	char message[SIZE];
	assembleMessage("tget",filename,message);
	int sendcommand= send(sockfd,message,strlen(message)+1,0);
	if(sendcommand<0){
		perror("error on sending tget command");
	}
	return sendcommand;
}

int create_connection(){
	struct sockaddr_in server_addr, new_addr;
	char *ip="0.0.0.0";
	int port =8884;
	int connecting;
	
	int sockfd;
	FILE *fd;

	char* filename="tst.txt";

	sockfd =socket(AF_INET,SOCK_STREAM,0);
	
	if(sockfd<0){
		perror("TCP socket failed");
		return -99;
	}

	server_addr.sin_family=AF_INET;
	server_addr.sin_port=port;
	server_addr.sin_addr.s_addr=inet_addr(ip);
	connecting = connect(sockfd,(struct sockaddr*)&server_addr, sizeof(server_addr));

	if(connecting<0){
		perror("error in connecting\n");
		return -99;
	}
	else{
		printf("Connected to server\n");
	}
	return sockfd;

}
void assembleMessage(char* command, char* option, char* result){
	strcpy(result,command);
	strcat(result, option);
}
void assembleTputMessage(char* command, char* option, unsigned long size, char* result ){
	assembleMessage(command,option,result);
	char temp[20];
	sprintf(temp,"%lu",size);
	strcat(result," ");
	strcat(result,temp);
}
/*****************************************************************************
 * Main Entry - Add any statup code required
 *****************************************************************************/
int main( int argc, char **argv ) 
{
	
  /*
   * Place any startup code you need here
   */

  /*
   * This is the call to the command line parser and tokenizer. You should not
   * need to modify anything within the library or the parser.  If you do, you 
   * will need to readup on the re2c 1.0.1 program.
   */
  	ProceessLoop();
  	return 0;
}
