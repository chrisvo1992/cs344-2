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
int isBuiltIn(struct Command*);
void* processCommand(int, struct Command*);
void runBuiltIn(int);
void checkRedirection(struct Command*);
void printError();

int main() 
{
	char* input = NULL;
	struct Command* list = NULL;
	while (1) {
		printShell();	
		input = parseInput();	
		if (isValid(input)) {
			list = createArgv(input);	
			processCommand(isBuiltIn(list), list);
		} else {
			printError();
		}	
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

void* processCommand(int builtIn, struct Command* cmd) {
	if (builtIn) {
		runBuiltIn(builtIn);
		return NULL;
	}
	checkRedirection(cmd);
	return NULL;
}

int isBuiltIn(struct Command* cmd) {
	if (strcmp(cmd->val, "exit") == 0) {
		return 1;
	}	
	if (strcmp(cmd->val, "status") == 0) {
		return 2;
	}	
	if (strcmp(cmd->val, "cd") == 0) {
		return 3;
	}	
	return 0;
}

void runBuiltIn(int type) {
	switch (type) {
		case 1:
			printf("exit\n");
		break;
		case 2:
			printf("status\n");
		break;
		case 3:
			printf("cd\n");
		break;
		default:
		break;
	}
}

void checkRedirection(struct Command *list) {
	printf("check for redirection\n");
	while (list != NULL) {
		printf("%s ",list->val);
		list = list->next;
	}
}

struct Command* createCommandNode(char* str) {
	struct Command* newCmd = (struct Command*)malloc(sizeof(struct Command));
	newCmd->val = calloc(strlen(str) + 1, sizeof(char));
	strcpy(newCmd->val, str);
	newCmd->next = NULL;
	return newCmd;
}

struct Command* createArgv(char* str) {
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
