#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

// set the shell script to smallsh
int main(void) {

	char smallsh[8] = "smallsh";

	char* newargv[] = { smallsh, NULL }; 

	setenv(smallsh, "smallsh", 1);

	execv(newargv[0], newargv);
	perror("execv");
	exit(EXIT_FAILURE);

	return 0;	
}
