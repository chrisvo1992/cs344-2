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

// man7.org/linux/man-pages/man7/ip.7.html/ refers to it.	
void setupAddressStruct(struct sockaddr_in* address, int port) {
	memset((char*) address, '\0', sizeof(*address));
	address->sin_family = AF_INET;
	address->sin_port = htons(port);
	//address->sin_addr.s_addr = INADDR_ANY;
	address->sin_addr.s_addr = "localhost";
}

int main(int argc, char* argv[]) {
	
	int connectingSocket, charsRead;
	char buffer[256];
	char* serverMsg = "I am the server,and I have recvd your message.";
	struct sockaddr_in serverAddress, clientAddress;
	socklen_t sizeOfClientInfo = sizeof(clientAddress);
	printf("client address struct: ", clientAddress);
	
	// if there are not enough arguments
	if (argc < 2) {
		fprintf(stderr, "usage: %s port\n", argv[0]);
		exit(1);
	}
	
	// create communication endpoint and return a fd to refer to
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	// must output an error if it cannot be run due to a network
	// error, such as the ports not being available
	if (serverSocket < 0) {
		error("ERROR opening socket");
	}

	// init the server socket with the provided port
	setupAddressStruct(&serverAddress, atoi(argv[1]));

	// assign the serverAddress to the serverSocket
	if (bind(serverSocket,
					(struct sockaddr *)&serverAddress,
					sizeof(serverAddress)) < 0) {
		error("ERROR on binding");
	}

	// passive socket that is now listening for incoming connections.
	// a max of 5 pending connections can be queued. If the queue is
	// full, an error with ECONNREFUSED. retransmission is possible,
	// lookup how to do that and mark this as done: [ ]
	listen(serverSocket, 5);

	while(1) {
		// create and return a new fd for the connectingSocket that the 
		// serverSocket is listening on. The sockets are now connected.
		connectingSocket = accept(serverSocket, 
								(struct sockaddr *)&clientAddress,
								&sizeOfClientInfo);
		if (connectingSocket < 0) {
			error("ERROR on accept");
		}	
		printf("SERVER: Connected to client running at host %d port %d\n",
																ntohs(clientAddress.sin_addr.s_addr),
																ntohs(clientAddress.sin_port));

		memset(buffer, '\0', 256);

		// return the length of the message on successful completion
		charsRead = recv(connectingSocket, buffer, 255, 0);

		if (charsRead < 0) {
			error("ERROR reading from socket");
		}

		printf("SERVER has recvd \"%s\"\n", buffer);

		charsRead = send(connectingSocket, &serverMsg, sizeof(serverMsg), 0);

		if (charsRead < 0) {
			error("ERROR writing to socket");
		} 
		close(connectingSocket);
	}

	close(serverSocket);

	return 0;
}
