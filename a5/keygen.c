#include <stdlib.h>
#include <stdio.h>
/*
 	creates a key file of a specified length. 
	characters will be any of the 27 chars, 
	including the space char.

	generated with random unix methods
	
	the first argument will specify the number
	of characters to generate in the key file


	/////////////////////
	CREATE THE CIPHERTEXT

  = message + key

	= (message+key)mod26

	= ciphertext

	////////////////////////
	READ THE CIPHERTEXT

 	= ciphertext - key

	= (ciphertext-key) mod26

	= message	

	////////////////////////
*/
int main(int argc, char* argv[]) {

	int length = atoi(argv[1]);

	if (argc < 2) {
		fprintf(stderr, "%s", "Not enough arguments\n");
		exit(1);
	}

	for (int i = 0; i < length; i++) {
		printf("%c", rand() % (90 - 65 + 1) + 65);	
	}
	printf("\n");

	return 0;
}
