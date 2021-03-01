#include <stdlib.h>
#include <stdio.h>
/*
 	creates a key file of a specified length. 
	characters will be any of the 27 chars, 
	including the space char.

	generated with random unix methods
	
	the first argument will specify the number
	of characters to generate in the key file
		
*/
int main(int argc, char* argv[]) {

	if (argc < 2) {
		fprintf(stderr, "%s", "Not enough arguments\n");
		exit(1);
	}

	for (int i = 0; i < argc; i++) {
		printf(" %d ", rand());	
		printf("");	
	}

	return 0;
}