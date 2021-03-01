#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) {
	perror(msg);
	exit(1);
}	

void setupAddressStruct(struct sockaddr_in* address,
												int port) {
	
	memset((char*) address, '\0', sizeof(*address));

	address->sin_family = AF_INET;
	address->sin_port = htons(port);
	address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char* argv[]) {
	
	int connectionSocket, charsRead;
	char buffer[256];
	struct sockaddr_in serverAddress, clientAddress;
	socklen_t sizeOfClientInfo = sizeof(clientAddress);
	
	if (argc < 2) {
		fprintf(stderr, "usage: %s port\n", argv[0]);
		exit(1);
	}

	int listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (listenSocket < 0) {
		error("ERROR opening socket");
	}
	
	setupAddressStruct(&serverAddress, atoi(argv[1]));

	if (bind(listenSocket,
					(struct sockaddr *)&serverAddress,
					sizeof(serverAddress)) < 0) {
		error("ERROR on binding");
	}
	listen(listenSocket, 5);

	while(1) {
		connectionSocket = accept(listenSocket, 
								(struct sockaddr *)&clientAddress,
								&sizeOfClientInfo);
		if (connectionSocket < 0) {
			error("ERROR on accept");
		}	
		printf("SERVER: Connected to client running at host %d port %d\n",
																ntohs(clientAddress.sin_addr.s_addr),
																ntohs(clientAddress.sin_port));

		memset(buffer, '\0', 256);

		charsRead = recv(connectionSocket, buffer, 255, 0);

		if (charsRead < 0) {
			error("ERROR reading from socket");
		}

		printf("SERVER has recvd \"%s\"\n", buffer);

		charsRead = send(connectionSocket,
									"I am the server,and I have recvd your message", 39, 0);

		if (charsRead < 0) {
			error("ERROR writing to socket");
		} 
		close(connectionSocket);
	}

	close(listenSocket);

	return 0;
}
