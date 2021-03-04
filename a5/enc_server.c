#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
} 

// get message from the client request.
// the flag separator is the # character 
char* parseText(char* buf) {
	char* str = NULL;
	int i = 4;
	int j = 0;
	char ch;
	// read past the enc_ part of the message by
	// initializing i to 4
	while (ch != '#') {
		ch = buf[i];
		i++;
	}

	// the length of the message will be one less due
	// to the ending # character. reset i and create the
	// message. ugly way to do it but whatever
	str = calloc(i - 4, sizeof(char));
	i = 4;		
	ch = ' ';
	while (ch != '#') {
		ch = buf[i];
		str[j] = ch;
		i++;
		j++;
	}
	str[j-1] = '\0';
	str[j] = '\0';
	printf("parseText: %s\n", str);
	return str;
}

// get key from the client request.
// the flag separator is the # character 
char* parseKey(char* buf) {
	char flag[] = "#";
	char* p;
	char* str;
	p = strstr(buf, flag);
	str = calloc(strlen(p) + 1, sizeof(char));	
	strcpy(str, p);
	return str;
}
// Checks the prefix to ensure that the enc_client
// is connecting to the enc_server. dec_client
// must not be able to connect.
char* checkPrefix(const char* str) {
	char needle[] = "enc_";
	char* p;
	p = strstr(str, needle);
	if (p && strlen(p) == strlen(str)) {
		return p;
	} else {
		p = NULL;
	} 
}

// perform the mod 27 on each valid character.
void mod27(char ch) {
	printf("%c", ch);
}

// If the character is not within the valid 
// range of [65, 90] or 32 for the space character
// then stop processing and return null
void createCipher(char* msg, char* key) {
	int c;
	printf("in mod27:: %s\n", msg);
	//return NULL;
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}


// 	requirements: must ensure that the child process created is 
// 	communicating with enc_client
//	input: listening_port as argv[1]
//	output: write backa  ciphertext to the enc_client that connected
//				to the server
int main(int argc, char *argv[]){
  int connectingSocket, charsRead;
  char buffer[256];
	char* response = "enc_server response\0";
	char* text;
	char* key;
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    exit(1);
  } 
  
  // Create the socket that will listen for connections
  int server = socket(AF_INET, SOCK_STREAM, 0);
  if (server < 0) {
    error("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(server, 
          (struct sockaddr *)&serverAddress, 
          sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(server, 5); 
  
  // Accept a connection, blocking if one is not available until one connects
  while(1){
    // Accept the connection request which creates a connection socket
    connectingSocket = accept(server, 
                (struct sockaddr *)&clientAddress, 
                &sizeOfClientInfo); 
    if (connectingSocket < 0){
      error("ERROR on accept");
    }

    printf("SERVER: Connected to client running at host %d port %d\n", 
                          ntohs(clientAddress.sin_addr.s_addr),
                          ntohs(clientAddress.sin_port));	

		printf("SERVER - host: %d, port: %d\n",
					ntohs(serverAddress.sin_addr.s_addr),
					ntohs(serverAddress.sin_port));

    // Get the message from the client and display it
    memset(buffer, '\0', 256);
    // Read the client's message from the socket
    charsRead = recv(connectingSocket, buffer, 255, 0); 
    if (charsRead < 0){
      error("ERROR reading from socket");
    }
    printf("SERVER - RECEIVED: %s", buffer);
		printf("\n");

		char* response = "Returned ciphertext\0";
	
		// if the request is from the enc_client	
		if (checkPrefix(buffer)) {
			// get the message and key from the client request
			text = parseText(buffer);
			key = parseKey(buffer);
			printf("the text into createCipher: %s\n", text);
			createCipher(text, key);
		}
    // Send the ciphertext back to the client
    charsRead = send(connectingSocket, 
                    response, strlen(response), 0); 

    if (charsRead < 0){
      error("ERROR writing to socket");
    }
    // Close the connection socket for this client
    close(connectingSocket); 
  }
  // Close the listening socket
  close(server); 
  return 0;
}
