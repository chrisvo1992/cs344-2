#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define MAX_CHARS 2048
#define MAX_ARGS 512


char* parseInput();
void printShell();

int main() 
{
	char* input = NULL;
	while (1) {
		printShell();	
		input = parseInput();	
	}
	return 0;
}

// takes input, checks the character and argument length
// returns: a character string of values
char* parseInput() {
	char* lineptr = NULL;
	size_t len = 0;
	size_t argc = 0;
	size_t lineCount = 0;

	lineCount =	getline(&lineptr, &len, stdin);

	if ((lineCount < MAX_CHARS)) {
		for (int i = 0; i < strlen(lineptr); i++) {
			if (lineptr[i] == 32 && lineptr[i] != 13) { argc++; }			
		}
		argc++;
		printf("number of arguments: %d\n", argc);
		if (argc <= MAX_ARGS) {
			return lineptr;
		}
	}
	return NULL;
}

// may need to redirect, or take control of,
// stdin stdout. this is why printf is in a 
// separate function
void printShell() {
	printf(": ");
}
