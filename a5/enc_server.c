#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define MAX_CONN 5
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
	str = calloc(i - 3, sizeof(char));
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
	return str;
}

// get key from the client request.
// the flag separator is the # character 
char* parseKey(char* buf) {
	char flag[] = "#";
	char* p;
	char* str;
	p = strstr(buf, flag);
	str = calloc(strlen(p), sizeof(char));	
	// get rid of the leading # character
	p++;
	strcpy(str, p);
	return str;
}

// Checks the prefix to ensure that the enc_client
// is connecting to the enc_server. dec_client
// must not be able to connect.
int checkPrefix(const char* str) {
	char needle[] = "enc_";
	char* p;
	p = strstr(str, needle);
	if (p && strlen(p) == strlen(str)) {
		return 1;
	} else {
		return 0;
	} 
}

// checks that the given character is within the 
// valid ranges of [65 - 90] or == 32
int checkRange(char ch) {
	int c = ch;
	int valid = 0;
	if (c == 32) {
		valid = 1;
	}
	if (c >= 65 && c <= 90) {
		valid = 1;
	}
	return valid;
}

// perform the mod 27 on each valid character in 
// the text and key.
// mod 26 bc of A = 0, Z = 25
char enc_mod27(char ch1, char ch2) {
	int c1 = ch1;
	int c2 = ch2;
	int mod;
	if (c1 == 32) {
		c1 = 26;
	} else {
		c1 -= 65;
	}
	if (c2 == 32) {
		c2 = 26;
	} else {
		c2 -= 65;
	}
	mod = c1 + c2;
	mod = mod % 27;
	//printf("(%i + %i) mod 26 = %i\n", c1, c2, mod);
	//printf("(%c + %c) mod 26 = %c\n\n", c1+65, c2+65, mod+65);
	mod += 65;
	if (mod == 91) { mod = 32; return mod; }
	return mod;
}

// If the character is not within the valid 
// range of [65, 90] or 32 for the space character
// then print message and keep processing 
char* createCipher(char* msg, char* key) {
	// str must be as long as the key to mask
	// the actual values if key > msg
	char* cipher = calloc(strlen(key), sizeof(char));
	for (int i = 0; i < strlen(msg); i++) {
		if (checkRange(msg[i])) {
			cipher[i] = enc_mod27(msg[i], key[i]);		
		} else {
			// just set it as a space char
			cipher[i] = 32;
			fprintf(stderr, "bad input\n");			
		}
	}	
	cipher[strlen(key)] = '\0';
	fflush(stdout);
	return cipher;
}

void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  memset((char*) address, '\0', sizeof(*address)); 

  address->sin_family = AF_INET;
  address->sin_port = htons(portNumber);
  address->sin_addr.s_addr = INADDR_ANY;
}

int sendall(int s, char *buf, int *len) {
	int total = 0;
	int bytesleft = *len;
	int n;

	while (total < *len) {
		n = send(s, buf+total, bytesleft, 0);
		if (n == -1) { break; }
		total +=n;
		bytesleft -= n;
	}
	*len = total;
	return n == -1 ? -1 : 0;
}

// 	requirements: must ensure that the child process created is 
// 	communicating with enc_client
//	input: listening_port as argv[1]
//	output: write backa  ciphertext to the enc_client that connected
//				to the server
int main(int argc, char *argv[]){
  int connectingSocket, charsRead, pidCount = 0;
	pid_t pid;
	int status;
  char buffer[4096];
	char* response = NULL;
	char* text;
	char* key;
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    exit(1);
  } 
  
  // Create the socket FD that will listen for connections
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

  // Start listening for connetions on FD. Allow up to 5 connections 
  // to queue up. server is the parent socket
  listen(server, 5); 

  while(1){
    connectingSocket = accept(server, 
                (struct sockaddr *)&clientAddress, 
                &sizeOfClientInfo); 
    if (connectingSocket < 0){
      error("ERROR on accept");
    }

		if ((pid = fork()) == 0) {

			//close(server);

			//waitpid(pid, &status, WNOHANG);

			pidCount++;

			while (pidCount != MAX_CONN) {
				/*
				printf("SERVER: Connected to client running at host %d port %d\n", 
												ntohs(clientAddress.sin_addr.s_addr),
												ntohs(clientAddress.sin_port));	*/
				// Get the message from the client and display it
				memset(buffer, '\0', 4096);
				// Read the client's message from the socket
				charsRead = recv(connectingSocket, buffer, 4096, 0); 
				if (charsRead < 0){
					error("ERROR reading from socket");
				}
				fflush(stdout);
				// if the request is from the enc_client	
				if (checkPrefix(buffer)) {
					// get the message and key from the client request
					text = parseText(buffer);
					key = parseKey(buffer);
					response = createCipher(text, key);
					// Send the ciphertext back to the client
					/*
					charsRead = send(connectingSocket, 
												response, strlen(response), 0); 
					*/
					int size = strlen(response);
					charsRead = sendall(connectingSocket, response, &size);
					memset(response, '\0', 4096);
					if (charsRead < 0){
						error("ERROR writing to socket");
					}
					pidCount--;
					//close(connectingSocket);
				} else {
					response = "400";
					charsRead = send(connectingSocket, 
												response, strlen(response), 0);
					memset(response, '\0', 4096);
					pidCount--;
					//close(connectingSocket);
				}
			}
		}
  }
	//close(server);
  return 0;
}
