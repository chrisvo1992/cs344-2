// file: dec_server.c
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
// the flag separator is the # character.
// counts the number of digits received 
char* parseText(char* buf, int* size) {
	char* str = NULL;
	int len = 0;
	int i = 4;
	int j = 0;
	int k = 0;
	char ch;
	int c = buf[i];

	// get the prepended message length
	while ((c >= 48) && (c <= 57)) {
		k++;
		len += (c - 48);
		i++;
		len *= 10;
		c = buf[i];
	}

	if (k > 1) { len /= 10; }
	// save the size in the argument size	
	*size = len;	

	// set the first char to a known valid
	// value to enter the next loops
	ch = buf[i];
	
	while (ch != '#') {
		ch = buf[i];
		i++;
	}

	// the length of the message will be one less due
	// to the ending # character. reset i and create the
	// message. ugly way to do it but whatever
	str = calloc(i - 3 - k, sizeof(char));
	i = 4 + k;		
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
	char needle[] = "dec_";
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
char dec_mod27(char ch1, char ch2) {
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
	mod = c1 - c2;
	mod = mod % 27;
	mod = (((c1 - c2) + 27) % 27) + 65;
	if (mod == 91) {mod = 32; return mod;}
	/*
 	printf("(%i - %i) mod 27 = %i, (%i,%c)\n", c1, c2, mod, 
		(c1 - c2) + 27, (((c1 - c2) + 27) % 27) + 65);
	*/
	return mod;
}

// If the character is not within the valid 
// range of [65, 90] or 32 for the space character
// then print message and keep processing 
char* readCipher(char* msg, char* key) {
	// str must be as long as the key to mask
	// the actual values if key > msg
	char* cipher = calloc(strlen(key), sizeof(char));
	for (int i = 0; i < strlen(msg); i++) {
		if (checkRange(msg[i])) {
			cipher[i] = dec_mod27(msg[i], key[i]);		
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
  int connectingSocket, charsRead, pidCount = 0, size, inSize;
	int status;
	pid_t pid;
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
  
  // Create the socket that will listen for connections
  int server = socket(AF_INET, SOCK_STREAM, 0);
  if (server < 0) {
    fprintf(stderr,"ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(server, 
          (struct sockaddr *)&serverAddress, 
          sizeof(serverAddress)) < 0){
    fprintf(stderr,"ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(server, 5); 
  
  // Accept a connection, blocking if one is not available until one connects
  while(1) {
    // Accept the connection request which creates a connection socket
    connectingSocket = accept(server, 
                (struct sockaddr *)&clientAddress, 
                &sizeOfClientInfo); 
    if (connectingSocket < 0){
      fprintf(stderr,"ERROR on accept");
    }

		if ((pid = fork()) == 0) {

			//close(server);
			//waitpid(pid, &status, WNOHANG);

			pidCount++;
				
			//while (1) {
			while (pidCount != MAX_CONN) {

				// Get the message from the client and display it
				memset(buffer, '\0', sizeof(buffer));
				// Read the client's message from the socket
				charsRead = recv(connectingSocket, buffer, sizeof(buffer), 0); 
				if (charsRead < 0){
					//fprintf(stderr,"ERROR reading from socket");
				}
				// if the request is from the enc_client	
				if (checkPrefix(buffer)) {
					// get the message and key from the client request
					text = parseText(buffer, &inSize);
					// assign the prepended message length and discard section
					// of message
					key = parseKey(buffer);
					response = readCipher(text, key);
					// Send the ciphertext back to the client
					/*
					charsRead = send(connectingSocket, 
												response, strlen(response), 0); 
					*/
					fflush(stdout);
					///*
					size = strlen(response);
					charsRead = sendall(connectingSocket, response, &size);
					//close(connectingSocket);
					//*/
					memset(response, '\0', sizeof(response));
					if (charsRead < 0){
						fprintf(stderr,"ERROR writing to socket");
					}
					pidCount--;
				} else {
					response = "400";
					size = strlen(response);
					charsRead = send(connectingSocket, response, strlen(response), 0);
					memset(response, '\0', sizeof(response));
					//close(connectingSocket);
					pidCount--;
				}
				close(connectingSocket);
			}
			//close(connectingSocket);
    }
		//close(connectingSocket);
  }
	//close(server);
  return 0;
}
