#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <signal.h>
#include <netdb.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues
void catchSIGSEGV(int);
int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[500000];
	struct sigaction SIGTSTP_action = {0};
  SIGTSTP_action.sa_handler = catchSIGSEGV;
  sigfillset(&SIGTSTP_action.sa_mask);
  SIGTSTP_action.sa_flags = 0;
  sigaction(SIGSEGV, &SIGTSTP_action, 0);

	if (argc < 2) { fprintf(stderr,"USAGE: %s <input-file> <key> <port>\n", argv[0]); exit(0); } // Check usage & args

  //Converts plaintext to C-string;
  FILE *fp;
  fp = fopen(argv[1], "r");
  if(fp == NULL){
    printf("ERROR: Bad plaintext file\n");
    exit(1);
  }
  char *pt;
  size_t len = 0;
  getline(&pt, &len, fp);

  //Converts key to C-string
  fp = fopen(argv[2], "r");
  if(fp == NULL){
    printf("ERROR: Bad key file\n");
    exit(1);
  }
  char *key;
  getline(&key, &len, fp);

  int cutoff = strlen(pt);

  char matchKey[cutoff];
  int i = 0;
  for(i = 0; i < cutoff; i++){
    matchKey[i] = key[i];
  }

  //Check if key is shorter than plaintext1
  if(strlen(key) < strlen(pt)){
    error("Key too short!.\n");
    exit(1);
  }
  //Check for bad characters
  for(i = 0; i < strlen(pt)-1; i++){
    if(pt[i] != 32 && pt[i] != 0){
      if( pt[i] > 90 || pt[i] < 65){
        error("Bad Characters. Only Capitals A-Z and spaces allowed\n");
        exit(1);
      }
    }
  }

  //Replace newlines with data divider characters
  pt[strcspn(pt, "\n")] = '/';
  matchKey[strcspn(matchKey, "\n")] = '#';

  //Concat to one string
  strcat(pt, matchKey);

  // printf("Data to send: %s\n", pt);


	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// // Send message to server
	charsWritten = send(socketFD, pt, strlen(pt), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");
	//
	// // Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	// charsRead = recv(socketFD, buffer, strlen(pt), 0); // Read data from the socket, leaving \0 at end
	// if (charsRead < 0) error("CLIENT: ERROR reading from socket");

	// bool notFound = true;
	// int totalSent = 0;
	// int count = 0;
	// int length = strlen(buffer);
	// do{
	// 	charsWritten = send(socketFD, &buffer[count], length, 0); // Write to the server
	// 	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	// 	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");
	// 	count += charsWritten;
	// 	length -= charsWritten;
	// 	for(i = 0; i < count; i++){
	// 		if(buffer[i] == '#'){
	// 			notFound = false;
	// 			break;
	// 		}
	// 	}
	//
	// }while(notFound);
	//
	//
	// memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	//
	bool notFound = true;
	int count = 0;
	do{
		charsRead = recv(socketFD, &buffer[count], 1, 0); // Read the client's message from the socket
		if (charsRead < 0) error("ERROR reading from socket");

		count += 1;
		for(i = 0; i < count; i++){
			if(buffer[i] == '#'){
				buffer[i] = '\0';
				notFound = false;
				break;
			}
		}

	}while(notFound);
	// printf("%s\n", buffer);

  // printf("%d\n", charsRead);
	printf("%s\n", buffer);
	fflush(stdout);
  // shutdown(socketFD, SHUT_RDWR);
	close(socketFD); // Close the socket

}

void catchSIGSEGV(int signo){
	// abort();
}
