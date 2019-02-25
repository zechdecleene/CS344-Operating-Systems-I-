#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues
void catchSIGSEGV(int);

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[500000];
	struct sockaddr_in serverAddress, clientAddress;
	struct sigaction SIGTSTP_action = {0};
  SIGTSTP_action.sa_handler = catchSIGSEGV;
  sigfillset(&SIGTSTP_action.sa_mask);
  SIGTSTP_action.sa_flags = 0;
  sigaction(SIGSEGV, &SIGTSTP_action, 0);
	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

  pid_t spawnPID = -5;
  int childExitStatus = -5;
  int bgpid[64];
  int numbg = 0;
	int exitcode, idx;
	int i = 0;


  while(1){

		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");

    spawnPID = fork();
		bgpid[numbg] = spawnPID;
		numbg++;
    switch (spawnPID) {
      case -1:{perror("HULL Breach!\n"); break; }
      case 0:
      {



				memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse

				bool notFound = true;
				int totalSent = 0;
				int count = 0;
				do{
					charsRead = recv(establishedConnectionFD, &buffer[count], 1, 0); // Read the client's message from the socket
	      	if (charsRead < 0) error("ERROR reading from socket");

					count += 1;
					for(i = 0; i < count; i++){
						if(buffer[i] == '#'){
							notFound = false;
							break;
						}
					}

				}while(notFound);


      	// Get the message from the client and display it
      	// charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer), 0); // Read the client's message from the socket
      	// if (charsRead < 0) error("ERROR reading from socket");

        // printf("DATA RECIEVED: %s\n", buffer);

        //Separate message from key
        int sizer = 0;
        for(sizer = 0; sizer < strlen(buffer); ++sizer){
          if(buffer[sizer] == '/'){
            break;
          }
        }

        char msg[sizer+1];
        memset(msg, '\0', sizeof(msg));
				i = 0;
        while(buffer[i] != '/'){
          msg[i] = buffer[i];
          i++;
        }
        i++; //Start Point for key


        //Create c-string for key
        char key[sizer+1];
        memset(key, '\0', sizeof(key));
        int loop = 2 * i;
        int keyidx = 0;
        while(i <= loop){
          key[keyidx] = buffer[i];
          i++;
          keyidx++;
        }

        //Encrypt message
        char encryption[sizer+3];
        memset(encryption, '\0', sizeof(encryption));

        for(i = 0; i < strlen(msg); i++){
          //Convert Ascii character to digits 0-26 for manipulation
          int msgval = (int) msg[i] - 65;
          int keyval = (int) key[i] - 65;
          if(msg[i] == 32){
            msgval = 26; //If msg is a space replace with 26
          }
          if(key[i] == 32){
            keyval = 26;    //if key is a space replace with 26
          }

          int newchar = msgval + keyval;
          if(newchar > 26){
            newchar = newchar - 27;
          }
          newchar += 65;
          if(newchar == 91){
            newchar = 32;
          }
          encryption[i] = (char) newchar;

					encryption[i+1] = '#';
        }
        // printf("(MSG)%c + (KEY)%c = (ENC)%c\n", msg[4], key[4], encryption[4]);
        // printf("(MSG)%d + (KEY)%d = (ENC)%d\n", msg[4], key[4], encryption[4]);

				// printf("%s\n\n", buffer);
				// printf("%s\n\n", key);

				// notFound = true;
				// totalSent = 0;
				// count = 0;
				// int length = strlen(encryption);
				// int charsWritten;
				// do{
				// 	charsWritten = send(establishedConnectionFD, &encryption[count], length, 0); // Write to the server
				// 	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
				// 	count += charsWritten;
				// 	length -= charsWritten;
				// 	for(i = 0; i < count; i++){
				// 		if(encryption[i] == '#'){
				// 			notFound = false;
				// 			break;
				// 		}
				// 	}
				//
				// }while(notFound);

        // Send a Success message back to the client
        charsRead = send(establishedConnectionFD, encryption, strlen(encryption), 0); // Send success back
        if (charsRead < 0) error("ERROR writing to socket");
      }
      default: {
				for(idx = 0; idx < numbg; idx++){
					waitpid(bgpid[idx], &childExitStatus, WNOHANG);
				}
      }

    }
  }
	close(establishedConnectionFD); // Close the existing socket which is connected to the client}
  close(listenSocketFD); // Close the listening socket
	return 0;
}

void catchSIGSEGV(int signo){
	abort();
}
