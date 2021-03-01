#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/*
	client code
	1  create a socket and connect to the server specified in the command arguments.
	2 prompt the user for input and send that input as a message to the server
	3 print the message received from the server and exit the program. 
*/

void error(const char *msg) {
	perror(msg);
	exit(0);
}

void setupAddressStruct(struct sockaddr_in* address, 
												int port, 
												char* hostname){

	// clear out the address struct
	memset((char*) address, '\0', sizeof(*address));
	address->sin_family = AF_INET;
	address->sin_port = htons(port);

	// get the DNS entry for this host name	
	struct hostent* hostInfo = gethostbyname(hostname);
	if (hostInfo == NULL) {
		fprintf(stderr, "client: ERROR, no such host\n");
		exit(0);
	}
	
	// copy the first ip address from the DNS entry to sin_addr.s_addr
	memcpy((char*) &address->sin_addr.s_addr,
					hostInfo->h_addr_list[0],
					hostInfo->h_length);
}

int main(int argc, char* argv[]) {
	
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;

	char buffer[256];
	// check usae and args

	if (argc < 3) {
		fprintf(stderr, "usage: %s hostname port\n", argv[0]);
		exit(0);
	}

	// create a socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD < 0) {
		error("CLIENT: ERROR opening socket");
	}

	setupAddressStruct(&serverAddress, atoi(argv[2]), argv[1]);
	if (connect(socketFD, 
							(struct sockaddr*)&serverAddress, 
							sizeof(serverAddress)) < 0) {
		error("client: error connecting");
	}

	printf("CLIENT: Enter text to send to the server: ");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer) - 1, stdin);
	buffer[strcspn(buffer, "\n")] = '\0';

	charsWritten = send(socketFD, buffer, strlen(buffer), 0);
	if (charsWritten < 0) {
		error("CLIENT: ERROR writing to socket");
	}

	if (charsWritten < strlen(buffer)) {
		printf("CLIENT: WARNING: Not all data written to socket\n");
	}

	memset(buffer, '\0', sizeof(buffer));

	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
	if (charsRead < 0) {
		error("CLIENT: ERROR reading from socket");
	}

	printf("CLIENT: recvd this \"%s\"\n", buffer);

	close(socketFD);

	return 0;
}
