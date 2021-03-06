#include <stdlib.h>
#include <stdio.h>
#include <time.h>
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

	srand(time(NULL));

	int length = atoi(argv[1]);
	int val;

	if (argc < 2) {
		fprintf(stderr, "%s", "Not enough arguments\n");
		exit(1);
	}
	// using 91 for the space character. if
	// the key char == 91, assign it to space
	// during encryption.
	// (upper - lower + 1) + lower
	for (int i = 0; i < length; i++) {
		val = (rand() % (91 - 65 + 1) + 65);
		if (val == 91) { val = 32; }	
		printf("%c", val);	
	}
	printf("\n");

	return 0;
}
