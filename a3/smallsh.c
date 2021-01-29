#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_LEN 2048
#define MAX_ARGS 512

void parse(char* cmd, char** val) {
	while(*cmd == ' ' || *cmd == '\t' || *cmd == '\n') {
		*cmd++ = '\0';
		*val++ = cmd;
		while(*cmd != '\0' && *cmd != ' ' && *cmd != '\t' && *cmd != '\n') {
			cmd++;
		}
		*val = '\0';
	}
}

void run_command(char** val) {
	pid_t pid;
	int status;

	pid = fork();

	printf("%s\n", *val);

	switch(pid) {
		case -1:
			perror("fork() failed");
			exit(1);
			break;
		case 0:
			if (execvp(*val, val) < 0) {
				perror("execvp");
				exit(EXIT_FAILURE);
			}
			break;
		default:
			pid = waitpid(pid, &status, 0);
			exit(0);
			break;
	}
}

int main() {

	char input[MAX_LEN + 1] = "";	

	// Enter the smallsh command line
	while(1) {
		printf(": ");
		// read the input, skipping white-space (space, tab, newline, etc.)
		if(scanf("%s", input) < 1) {
			printf("Yea, no...\n");
		}
		printf("%s\n", input);
		//parse(input, argv);
		//run_command(argv);	
	}
	return 0;
}
