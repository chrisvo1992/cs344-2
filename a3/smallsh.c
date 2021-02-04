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
// ouput: 
// 	1: exit 
// 	2: cd
// 	3: status
// 	4: all others
int checkCommand(struct node *cmd)
{
	if(strcmp(cmd->val, "exit") == 0)
	{
		return 1;
	}

	if(strcmp(cmd->val, "cd") == 0)
	{
		return 2;
	}

	if(strcmp(cmd->val, "status") == 0)
	{
		return 3;
	}

	// all others
	return 4;
}

///////////////////////////////////////////////////////////////////////////////
// run as foreground process
// input: pointer to a list of null-teminated strings
// output: pid status
int runForeground(char** arg)
{
	int childStatus;
	pid_t spawnPid = fork();
	switch(spawnPid)
	{
		case -1:
			perror("fork failed");
			exit(1);	
			break;
		case 0:
			printf("child(%d) running %s\n", getpid(), arg[0]);
			execvp(arg[0], arg);
			perror("execvp");
			exit(1);		
			break;
		default:
			spawnPid = waitpid(spawnPid, &childStatus, 0);
			//printf("parent(%d): child(%d) terminated.\n", getpid(), spawnPid);
			break;
	}
	return spawnPid;
}

///////////////////////////////////////////////////////////////////////////////
// run as background process
// input: pointer to a list of null-teminated strings
// output: pid status
int runBackground(char** arg)
{
	int childStatus;

	pid_t spawnPid = fork();
	spawnPid = waitpid(-1, &childStatus, WNOHANG);

	if (spawnPid < 0) 
	{
		perror("fork failed");
		exit(1);	
	}
	else if (spawnPid > 0)
	{
		printf("child(%d) running %s\n", getpid(), arg[0]);
		execvp(arg[0], arg);
		perror("execvp");
		exit(1);		
		printf("spawnPid %d\n", spawnPid);
	}
	return spawnPid;
}
///////////////////////////////////////////////////////////////////////////////
// constructs the argument list from the array of pointers to 
// null-terminated strings
// input: the head of the linked list of commands
// output: stdout
void processBashCommands(struct node* cmd)
{
	char *argv[3];
	char cmdOpts[MAX_LEN] = "";
	char* cmdStr = malloc((strlen(cmd->val) + 1) * (sizeof(char)));
	strcpy(cmdStr, cmd->val);
	cmd = cmd->next;
	argv[0] = cmdStr;
	int flag = 0;

	while(cmd != NULL)
	{
		flag = 1;
		strcat(cmdOpts, cmd->val);
		strcat(cmdOpts, "\0");
		cmd = cmd->next;
	}

	if (cmdOpts[strlen(cmdOpts) - 1] == '&')
	{
		cmdOpts[strlen(cmdOpts) - 1] = '\0';
		flag = 2;
	}

	//printf("%s %i\n", cmdOpts, strlen(cmdOpts));
	
		// check if there are options or nah.
	if (flag){ argv[1] = cmdOpts; argv[2] = NULL; }
	else { argv[1] = NULL; argv[2] = NULL; }
	
	// choose whether it is a foreground or background process
	// printf("%c\n", cmdOpts[strlen(cmdOpts) - 1]);
	if (flag == 2)
	{
		runBackground(argv);
	}
	else 
	{
		runForeground(argv);	
	}
}

///////////////////////////////////////////////////////////////////////////////
// check the kind of command to run. if it is 4, process differently
// 	1: exit 
// 	2: cd
// 	3: status
// 	4: all others
// input: a list of commands 
// ouput: the result of those commands
void peek_commands(struct node* cmds) 
{
	char *temp = NULL;
	char *home = getenv("HOME");
	struct node* head = cmds;
	int cmdType = checkCommand(head);	
	
	switch(cmdType)
	{
		case 1:
			// if pid == 0, sig is sent to every process in the process group
			// https://man7.org/linux/man-pages/man2/kill.2.html
			// https://man7.org/linux/man-pages/man7/signal.7.html
			kill(0, SIGKILL);
			exit(0);
		break;
		case 2:
		//https://man7.org/linux/man-pages/man3/getenv.3.html
			if (head->next == NULL )
			{
				if (chdir(home) < 0)
				{
					perror("HOME");
				}
			}
			else
			{
				if (chdir(head->next->val) < 0)
				{
					perror(head->next->val);
				}
			}
			///*// can comment out at the end
			temp = get_current_dir_name();
			printf("%s\n", temp);
			free(temp);
			//*/		
		break;
		case 3:
			printf("print the exit status or the last terminal signal of the");
			printf("last foreground process, whatever tf that means.\n");	
		break;
		case 4:
			processBashCommands(head);		
		break;
		default:
		break;
	}
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
		peek_commands(commands);
		destroyCommandList(commands);
		fflush(stdin);
		strcpy(*input, "");
	}

	return 0;
}
