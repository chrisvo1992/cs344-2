#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_NARGS      		512
#define MAX_INPUT_LENGTH    2048

int bgToggle;
int sigtstpFlag;
int sigintFlag;

//custom handler for SIGTSTP, toggles foreground-only mode, prints text noteice, and raises signal flag
void handle_SIGTSTP(int signo){
	if (bgToggle == 0){
		char* message = "\nEntering foreground-only mode (& is now ignored)\n";
		bgToggle = 1;
		write(STDOUT_FILENO, message, 50);
	}
	else{
		char* message = "\nExiting foreground-only mode\n";
		bgToggle = 0;
		write(STDOUT_FILENO, message, 30);
	}
	sigtstpFlag = 1;
	fflush(stdout);
}

void runShell(char** argList, int *nArgs) {
	int bgFlag = 0;
	int status;
	int redirect;
	int FD;
	int i;
	pid_t pid;
	struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0};

	//ignore sigint signals
	SIGINT_action.sa_handler = SIG_IGN;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0;
	sigaction(SIGINT, &SIGINT_action, NULL);

	//custom SIGTSTP handler
	SIGTSTP_action.sa_handler = handle_SIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = 0;
  	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	pid - getpid();

		//check if last char is '&' and raise background flag and delete arg if true
		if (strcmp(argList[*nArgs - 1], "&") == 0) {
			(*nArgs)--;
			argList[*nArgs] = NULL;
			if (bgToggle == 0){
				bgFlag = 1;
				(*nArgs)++;
			}
		}

		//if exit command
		if (strcmp(argList[0], "exit") == 0) {
			fflush(stdout);
			while (1){
				exit(0);
			}
		} 

		//if change directory command
		else if (strcmp(argList[0], "cd") == 0) {
			if (*nArgs == 1)
				chdir(getenv("HOME"));
			else
				chdir(argList[1]);
		}

		//if status command
		else if (strcmp(argList[0], "status") == 0){
			if (WIFEXITED(status)){
				printf("exit value %d\n", WEXITSTATUS(status));
			}
			else if (WIFSIGNALED(status)){
				printf("terminated by signal %d\n", WTERMSIG(status));
			}
		}
		
		else{
			//for all other commands, spawn child
			pid = fork();
			switch (pid){
				case -1:
					perror("fork() failed\n");
					exit(1);
					break;
				case 0: //child stuff
					//custom sa handler for foreground
					if (bgFlag == 0){
						SIGINT_action.sa_handler = SIG_DFL;
						sigaction(SIGINT, &SIGINT_action, NULL);
					}
					
					for (i = 1; i < *nArgs; i++) {
						//if file not specified, pass io to /dev/null
						if (bgFlag == 1) {	
							redirect = 1;
							FD = open("/dev/null", O_RDONLY);
							int result = dup2(FD, STDIN_FILENO);
							if (result == -1) { 
								perror("source open()"); 
								exit(1); 
							}
						}
						// input file direction
						if ((strcmp(argList[i], "<") == 0)) {
							redirect = 1;
							
							FD = open(argList[i + 1], O_RDONLY);
							int result = dup2(FD, STDIN_FILENO);
							if (result == -1) { 
								perror("source open()"); 
								exit(1); 
							}
						}
						// output file direction
						if ((strcmp(argList[i], ">") == 0))  {
							redirect = 1;
							FD = open(argList[i + 1], O_CREAT | O_RDWR | O_TRUNC, 0644);
							int result = dup2(FD, STDOUT_FILENO); //should have made a function for all this repeated code 
							if (result == -1){
								perror("source open()"); 
								exit(1); 
							}
						}
					}
					//truncate arguments in preperation for exec and close FD
					if (redirect == 1){
						close(FD);
						for(i = 1; i < *nArgs; i++)
							argList[i] = NULL;
					}

					//pass trunceted arguments to execvp
					if (execvp(argList[0], argList) && sigtstpFlag != 1 && sigintFlag != 1){
						fprintf(stderr, "I do not understand %s\n", argList[0]);
						exit(1);
					}
					break;

				default: //parent stuff
					if (bgFlag == 1){
						printf("background pid is %d\n", pid);
					}
					else { 
						//wait for child to terminate
						waitpid(pid, &status, 0);
						if(sigtstpFlag != 1){
							//check that child is dead
							if (WIFSIGNALED(status) == 1 && WTERMSIG(status) != 0){
								printf("terminated by signal %d\n", WTERMSIG(status));
							}
							while (pid != -1){
								//re-fetch pid
								pid = waitpid(-1, &status, WNOHANG);	
								//print after results killed
								if (WIFEXITED(status) != 0 && pid > 0){
									printf("background pid %d is done: exit value %d\n", pid, WEXITSTATUS(status));
								}
								else if (WIFSIGNALED(status) != 0 && pid > 0 && bgToggle == 0){
									printf("background pid %d is done: terminated by signal %d\n", pid, WTERMSIG(status));
								}
							}
						}
					}
				
				break;
			}
		}

}

// fetches arguments from user input, parses into list and tallys, 
void getCommand(char** argList, int *nArgs) {
	char inBuff[MAX_INPUT_LENGTH];
	int i;
	char* argKey;
	char pid[12];

	do {
		printf(": ");
		fgets(inBuff, MAX_INPUT_LENGTH, stdin);
		strtok(inBuff, "\n"); // get rid of newline from fgets
	} while(inBuff[0] == '#'|| strlen(inBuff) < 1);

	//expand $$ to pid of shell
    for (i = 0; i < strlen(inBuff); i ++) {
		// swapping $$ for %d so we can sprintf pid in
		if ( (inBuff[i] == '$')  && (inBuff[i + 1] == '$') && (i + 1 < strlen(inBuff))) {
			char * temp = strdup(inBuff);
			temp[i] = '%';
			temp[i + 1] = 'd';
			sprintf(inBuff, temp, getpid());
			free(temp);
		}
	}

	// Parse user input into array of arguments
	argKey = strtok(inBuff, " ");	
	while(argKey != NULL) {
		argList[*nArgs] = strdup(argKey);
		(*nArgs)++;
		argKey = strtok(NULL, " ");
	}
}

int main() {
	int nArgs;
	char* argList[MAX_NARGS];
	bgToggle = 0;

	while (1) {
		//reset inputs
		nArgs = 0;
		sigtstpFlag = 0;
		sigintFlag = 0;
		memset(argList, '\0', MAX_NARGS);
		fflush(stdout);
		fflush(stdin);

		// Prompt user command
		getCommand(argList, &nArgs);
		// Execute command
		runShell(argList, &nArgs);
	}
	
	return 0;
}
