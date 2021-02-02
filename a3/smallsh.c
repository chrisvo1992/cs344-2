#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

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
	// this is just to null terminate the last char in the string
	// going on 3 hours of sleep but this is the easiest way that I see,
	// right now, to remove the newline character fromt *str. Might be
	// overkill and/or there might be a better way but this works for me
	// at this moment.
	unsigned int strLen = (unsigned int)strlen(*str);
	char *line = malloc(sizeof(strLen * sizeof(char)));
	strcpy(line, *str);
	line[strLen - 1] = '\0';
	
	/*// could use gdb for this but im here anyways	
	for (int i = 0; i < strLen; ++i) {
		printf("%c ", line[i]);
	}
	*/

	char* token = strtok_r(line, " ", &savePtr);

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

	free(line);

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
// check the commands. this is given a node taht can reference
// previous and next commands for things such as cd
// input: a node representing a command node
// ouput: the result of the command
//void checkCommand(char* str) 
void checkCommand(struct node *cmd)
{
	char *temp = NULL;
	char *home = getenv("HOME");

	//printf("%s ", str);
	///*
	if(strcmp(cmd->val, "exit") == 0)
	{
		printf("you entered %s\n", cmd->val);
		// if pid == 0, sig is sent to every process in the process
		// group.
		// https://man7.org/linux/man-pages/man2/kill.2.html
		// https://man7.org/linux/man-pages/man7/signal.7.html
		kill(0, SIGKILL);
		exit(0);
	}
	else if(strcmp(cmd->val, "cd") == 0)
	//https://man7.org/linux/man-pages/man3/getenv.3.html
	{
		if (cmd->next == NULL )
		{
			if (chdir(home) < 0)
			{
				perror("HOME");
			}
		}
		else
		{
			if (chdir(cmd->next->val) < 0)
			{
				perror("Yeah, no.");
			}
		}
		///*// can comment out at the end
		temp = get_current_dir_name();
		//*/

		printf("%s\n", temp);
	}
	else if(strcmp(cmd->val, "status") == 0)
	{
		printf("you entered %s\n", cmd->val);
	}
	else
	{
		printf("use exec family of functions\n");
	}
	//*/
}

///////////////////////////////////////////////////////////////////////////////
// for each command in the list, check them and run processes
// input: a list of commands
// ouput: the result of those commands
void run_commands(struct node* cmds) 
{
	struct node* head = cmds;
	pid_t pid;
	int status;

	while(head != NULL)
	{
		checkCommand(head);	
		/*
		for (int i = 0; i < strlen(head->val); ++i)
		{
			printf("%c", head->val[i]);
		}
		printf(" ");
		*/
		head = head->next;
	}

	/*
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
//
int main() 
{
	//char input[MAX_LEN + 1] = "";	
	char **input = malloc(sizeof(char) * MAX_LEN + 1);
	struct node* commands = NULL;
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
