#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 2048
#define MAX_ARGS 512

///////////////////////////////////////////////////////////////////////////////
// a linked list that represents each command entered on a single line
// type defined to CMD
struct node 
{
	char* val;
	struct node* prev;
	struct node* next;
};
//typedef node CMD;

///////////////////////////////////////////////////////////////////////////////
// create a command for a doubly linked list of commands.
// input: a token representing each command that has been 
// 	separated by a space.
// output: a node representing an individual command.
struct node* CreateCommandNode(char* str) 
{
	struct node* newCommand = (struct node*)malloc(sizeof(struct node));
	newCommand->val = calloc(strlen(str), sizeof(char));
	strcpy(newCommand->val, str);
	newCommand->prev = NULL;
	newCommand->next = NULL;

	//printf("%s\n", newCommand->val);

	return newCommand;
}

///////////////////////////////////////////////////////////////////////////////
// create a command for a doubly linked list of commands
void CreateCommandList(char* str, int* cmdCount) 
{
	struct node* newCmd;
	struct node* head = NULL;
	struct node* tail = NULL;
	struct node* curr;
	char* savePtr;
	
	//printf("Number of commands entered: %d\n", *cmdCount);

	char* token = strtok_r(str, " ", &savePtr);

	newCmd = CreateCommandNode(token); 
	
	tail = newCmd;
	head = tail;

	while(token) 
	{
		token = strtok_r(NULL, " ", &savePtr);
		newCmd = CreateCommandNode(token); 
		//printf("%s\n", newCmd->val);
		curr = tail;
		tail = newCmd;
		curr->next = tail;
		tail->next = NULL;	
	}

	/*
	while(head != NULL)
	{
		printf("%s\n", head->val);
		head = head->next;
	}
	*/
}


///////////////////////////////////////////////////////////////////////////////
void parse(char* str) 
{
	printf("%s\n", str);	

}

///////////////////////////////////////////////////////////////////////////////
void run_command(char** val) 
{
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

///////////////////////////////////////////////////////////////////////////////
int main() 
{
	char input[MAX_LEN + 1] = "";	
	int count = 0;

	// Enter the smallsh command line
	while(1) {
		printf(": ");
		// read the input, skipping white-space (space, tab, newline, etc.)
		count = scanf("%s", input);
		if(count < 1) {
			printf("Yea, no...\n");
		}
		CreateCommandList(input, &count);
	}

	return 0;
}
