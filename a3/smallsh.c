#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 2048
#define MAX_ARGS 512

///////////////////////////////////////////////////////////////////////////////
// a linked list that represents each command entered on a single line.
// created a doubly linked list to be able to both allocate mem on the 
// fly and check the prev and next commands of a current command input. 
struct node 
{
	char* val;
	struct node* prev;
	struct node* next;
};

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
// input: a char str of commands 
// output: a linked list of commands.
struct node* CreateCommandList(char** str) 
{
	struct node* newCmd;
	struct node* head = NULL;
	struct node* tail = NULL;
	struct node* curr;
	char* savePtr;
	
	char* token = strtok_r(*str, " ", &savePtr);

	while(token) 
	{
		newCmd = CreateCommandNode(token); 
		if (tail == NULL)
		{
			newCmd = CreateCommandNode(token); 
			tail = newCmd;
			head = tail;
			head->prev = NULL;
			tail->next = NULL;
		}
		curr = tail;
		tail = newCmd;
		curr->next = tail;
		tail->next = NULL;	
		tail->prev = curr;
		token = strtok_r(NULL, " ", &savePtr);
	}

	return head;
}
///////////////////////////////////////////////////////////////////////////////
// takes a list of commands and destroys them.
// input: linked list of commands
// ouput: destruction
void destroyCommandList(struct node* list)
{
	struct node* temp;
	while(list != NULL)
	{
		free(list->val);
		temp = list->next;
		free(list);
		list = temp;
	}
}

///////////////////////////////////////////////////////////////////////////////
void parse(char* str) 
{
	/*
	while(head != NULL)
	{
		printf("%s\n", head->val);
		head = head->next;
	}
	*/
}

///////////////////////////////////////////////////////////////////////////////
void run_commands(struct node* cmds) 
{
	struct node* head = cmds;

	while(head != NULL)
	{
		for (int i = 0; i < strlen(head->val); ++i)
		{
			printf("%c", head->val[i]);
		}
		printf(" ");
		head = head->next;
	}

	/*
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
	*/
}

///////////////////////////////////////////////////////////////////////////////
int main() 
{
	//char input[MAX_LEN + 1] = "";	
	char **input = malloc(sizeof(char) * MAX_LEN + 1);
	struct node* commands = NULL;
	ssize_t nread = 0;
	size_t len;

	// Enter the smallsh command line
	while(1) {
			printf(": ");
		// read the input
		if(getline(input, &len, stdin) == -1) {
			printf("Yea, no...\n");
			perror("getline");
		}
		commands = CreateCommandList(input);
		run_commands(commands);
		destroyCommandList(commands);
		fflush(stdin);
		strcpy(*input, "");
	}

	return 0;
}
