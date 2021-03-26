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

struct Command {
	char* val;
	struct Command* next;
};

char* parseInput();
void printShell();
int isValid(char*);
struct Command* createArgv(char*);
struct Command* createCommandNode(char*);
int checkCmd(struct Command*);
int processCommand(int, struct Command*, int);
int runCmd(int, struct Command*, int);
char* checkVarExp(char*);
char* expandVariable(char*);
char* pid_to_string();
void checkRedirection(struct Command*);
void printError();
int createProcess(struct Command*, int);

int main() 
{
	char* input = NULL;
	struct Command* list = NULL;
	int status;
	while (1) {
		printShell();	
		input = parseInput();	
		if (isValid(input)) {
			list = createArgv(input);	
			status = processCommand(checkCmd(list), list, status);
		} else {
			printError();
		}	
	}
	return 0;
}

int createProcess(struct Command* list, int stat) {
	printf("create process\n");
	
	pid_t spawn = fork();	
	int status = 0;

	switch (spawn) {
		case -1:
			perror("fork failed");
			status = 1;
			exit(1);
		break;
		case 0:
			status = stat + 1;
		break;
	}
	return status;
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
			if (lineptr[i] == 32) { argc++; }			
		}
		argc++;
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

int isValid(char* line) {
	if (line == NULL) {return 0;}
	return 1;
}

// takes the command flag, argument list, and the 
// most recent status of the last run command
int processCommand(int builtIn, struct Command* cmd, int stat) {
	// the command is builtin
	if (builtIn) {
		return runCmd(builtIn, cmd, stat);
	}
	checkRedirection(cmd);
 	// the command is not builtin
	return runCmd(builtIn, cmd, stat);
}

int checkCmd(struct Command* cmd) {
	if (strcmp(cmd->val, "exit") == 0) {
		return 1;
	}	
	if (strcmp(cmd->val, "cd") == 0) {
		return 2;
	}	
	if (strcmp(cmd->val, "status") == 0) {
		return 3;
	}	
	return 0;
}

// returns the exit status of non-builtin commands
int runCmd(int type, struct Command* list, int stat) {
	char* home = getenv("HOME");
	struct Command* head = list;
	int status;

	switch (type) {
		case 0: 
			status = createProcess(list, stat);
		break;
		case 1:
			status = 0;
			exit(0);
		break;
		case 2:
			if (head->next == NULL) {
				if (chdir(home) < 0) {
					perror("cd");
				}
			} else {
				if (chdir(head->next->val) < 0) { 
					perror(head->next->val);
				}
			}	
			status = 0;
		break;
		case 3:
			printf("exit value %d\n", stat);
		break;
		default:
		break;
	}
	return status;
}

// returns the location of '$$' in str found in 
// the list of command line arguments
char* checkVarExp(char* str) {
	char* s = strstr(str, "$$");
	if (s) {
		return s;
	}	
	return NULL;	
}

// concats the strings preceding and following the '$$'
// found in tkn
char* expandVariable(char* tkn) {
	int len = 0;	
	int i = 0;
	char c = tkn[len];
	char* str1 = NULL;
	char* str2 = NULL;
	char* pid = pid_to_string();
	char* str = NULL;
	while (c != '$') {
		len++;
		c = tkn[len];	
	}
	str1 = calloc(len + 1, sizeof(char));
	str2 = calloc(strlen(tkn) + 1, sizeof(char));
	strncpy(str1, tkn, len);

	// get the string following '$$'
	len += 2;
	while (len < strlen(tkn)) {
		str2[i] = tkn[len];
		i++;
		len++;	
	}
	str2[i] = '\0';

	// concat all three strings
	str = calloc(strlen(str1) + 
											strlen(str2) + 
											strlen(pid) + 1, sizeof(char));	
	strcat(str, str1);
	strcat(str, pid);
	strcat(str, str2);
	str[strlen(str)] = '\0';
	
	free(str1);
	free(pid);
	free(str2);
	return str;
}

// converts a current pid to a string
char* pid_to_string() {
	char* str = NULL; 
	pid_t pid = getpid();
	size_t digit = pid, rem = 0, num = digit, len = 0;

	if (pid > 0) {
		while (digit != 0) {
			len++;
			digit /= 10;
		}
		str = calloc(len + 1, sizeof(char));
		for (int i = 0; i < len; i++) {
			rem = num % 10;
			num = num / 10; 
			str[len - (i + 1)] = rem + '0';	
		}
	}
	return str;
}

void checkRedirection(struct Command *list) {
	printf("check for redirection\n");
}

struct Command* createCommandNode(char* str) {
	struct Command* newCmd = (struct Command*)malloc(sizeof(struct Command));
	newCmd->val = calloc(strlen(str) + 1, sizeof(char));
	strcpy(newCmd->val, str);
	newCmd->next = NULL;
	return newCmd;
}

struct Command* createArgv(char* str) {
	char* expansion_loc;
	char* pid_str = NULL;
	char* line = calloc(strlen(str) + 1, sizeof(char));
	strcpy(line, str);
	line[strlen(line)-1] = '\0';
	struct Command* argv = NULL;
	struct Command* temp = NULL;
	struct Command* head = NULL;
	struct Command* cmd = NULL;
	char* saveptr = NULL;

	char* token = strtok_r(line, " ", &saveptr);
	head = createCommandNode(token);
	argv = head;
	token = strtok_r(NULL, " ", &saveptr);
	while (token) {
		expansion_loc	= checkVarExp(token);		
		// replace the token with the pid, converted to a string
		if (expansion_loc != NULL){
			pid_str = expandVariable(token);
			memset(token, '\0', strlen(token) + 1);
			strcpy(token, pid_str);
			free(pid_str);
			token[strlen(token)] = '\0';
		} 
		temp = head;
		cmd = createCommandNode(token);	
		temp->next = cmd;
		head = cmd;
		cmd = NULL;
		token = strtok_r(NULL, " ", &saveptr);
	}
	head->next = NULL;
	return argv;
}

void printError() {
	printf("Too many characters/arguments\n");
}
