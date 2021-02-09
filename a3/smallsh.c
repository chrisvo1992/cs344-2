#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
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

struct proc 
{
	pid_t pid; 
	struct proc* next;
};


///////////////////////////////////////////////////////////////////////////////
// create a command for a doubly linked list of commands.
// input: a token representing each command that has been 
// 	separated by a space.
// output: a node representing an individual command.
struct node* CreateCommandNode(char* str) 
{
	struct node* newCommand = (struct node*)malloc(sizeof(struct node));
	newCommand->val = calloc(strlen(str) + 1, sizeof(char));
	strcpy(newCommand->val, str);
	// havent used prev but...its there
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

	// prevent segfault when user only enters newline chars
	if (*str[0] == '\n' || *str[0] == '\t' || *str[0] == '\r' || *str[0] == '#') 
	{	
		return NULL;
	}

	// this is just to null terminate the last char in the string
	// going on 3 hours of sleep but this is the easiest way that I see,
	// right now, to remove the newline character fromt *str. Might be
	// overkill and/or there might be a better way but this works for me
	// at this moment.
	unsigned int strLen = (unsigned int)strlen(*str);
	char *line = calloc(strLen + 1, sizeof(char));
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
			// doesnt seem as if prev is needed because of the string processing
			// being done but its here just in case.
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
//////////////////////////////////////////////////////////////////////////////
// checks if there are any redirect symbols
// input: linked list of commands
// output: true or false
int redirection(struct node* cmd)
{
	int fd;
	int redirect = 0;

	while (cmd != NULL)
	{
		if (strcmp(cmd->val, "<") == 0)
		{
			redirect = 1;
			// check for the next input to use for stdin
			//printf("trying current dir, file: %s\n", cmd->next->val);
			//fflush(stdin);
			if (cmd->next != NULL)
			{
				fd = open(cmd->next->val, O_RDONLY);
				//fcntl(fd, F_SETFD, FD_CLOEXEC);
			}
			else
			{
				fd = 0;
			}

			if (fd > 0)
			{
				//printf("duping\n");
				int result = dup2(fd, STDIN_FILENO);		
				if (result < 0) 
				{
					perror("dup2()");
					exit(1);
				}
			}
			else 
			{
				perror(cmd->next->val);
			}
		}	

		if (strcmp(cmd->val, ">") == 0)
		{
			redirect = 1;
			// check for the next input to use for stdout
			if (cmd->next != NULL)
			{
				//printf(cmd->next->val);
				//printf("\n");
				/*
				for (int i = 0; i < strlen(cmd->next->val); ++i)
				{
					printf("%c ", cmd->next->val[i]);
				}
				*/
				fd = open(cmd->next->val, O_WRONLY | O_CREAT | O_TRUNC, 0644);
				fcntl(fd, F_SETFD, FD_CLOEXEC);
			}
			else 
			{
				fd = 0;	
			}
			// if the file can be opened for stdout
			if (fd > 0)
			{
				//printf("duping\n");
				dup2(fd, STDOUT_FILENO);		
			}
			else 
			{
				perror(cmd->next->val);	
			}
		}	
		cmd = cmd->next;	
	}
	close(fd);
	return redirect;
} 

///////////////////////////////////////////////////////////////////////////////
// run as foreground process
// input: pointer to a list of null-teminated strings
// output: child status
int runForeground(char** arg, char* opts, struct node* cmd, int childStatus)
{
	int std_out = dup(STDOUT_FILENO);
	int std_in = dup(STDIN_FILENO);
	int* status = &childStatus;
	pid_t spawnPid = fork();
	int argCount = 0;
	int in_or_out = 0;

	// probably will need to pass in arg count from main
	///*
	if (argCount < MAX_LEN)
	{
		switch(spawnPid)
		{
			case -1:
				perror("fork failed");
				exit(1);	
				break;
			case 0:
				// get to the in and out files
				in_or_out = redirection(cmd);
				/*
				if (in_or_out) 
				{
					char* str = calloc(strlen(arg[0] + 1), sizeof(char));
					strcpy(str, arg[0]);
					strcat(str, "\0");
					strcat(str, opts);				
					strcat(str, "\0");	
					execlp(arg[0], str, (char*)NULL);
					free(str);
					perror(arg[0]);
					_exit(1);
				}
				else 
				{
					execvp(arg[0], arg);
					perror(arg[0]);
					_exit(1);
				}
				*/
				///*
				if (in_or_out)
				{
					execlp(arg[0], arg[0], NULL);
					perror(arg[0]);
					_exit(1);
				}
				else
				{
					execvp(arg[0], arg);
					perror(arg[0]);
					_exit(1);
				}
				//*/
				dup2(std_in, STDIN_FILENO);
				dup2(std_out, STDOUT_FILENO);
				//close(fd); // using fcntl
				break;
			default:
				spawnPid = waitpid(spawnPid, status, 0);
				break;
		}
	}	
	else 
	{
		printf("Max argument limit reached.\n");
		return 0;
	}
	return 0; 
}

///////////////////////////////////////////////////////////////////////////////
// run as background process. checks if there are any current background
// processes running. If there are not, creates a list of bg procs. 
// In either case, continues to check for the termination of any bg 
// process in the list.
// input: pointer to a list of null-teminated strings and a pointer to a
// list of background processes
// output: pid status
int runBackground(char** arg, char* opts, struct node* cmd)
{
	int std_out = dup(STDOUT_FILENO);
	int std_in = dup(STDIN_FILENO);
	int status;
	int in_or_out = redirection(cmd);
	pid_t spawnPid;
	spawnPid = fork();

	if (spawnPid < 0)
	{
		perror("fork failed");
		_exit(1);
	}
	else if (spawnPid == 0) 
	{
		printf("process(%i) running\n", getpid());
		fflush(stdout);
		if (in_or_out) 
		{
			/*
			int garbage = open("g.dat", O_WRONLY | O_TRUNC);
			fcntl(garbage, F_SETFD, FD_CLOEXEC);
			if (garbage < 0)
			{
				perror("open() failed");	
			}
			else 
			{
				dup2(garbage, STDOUT_FILENO);
			}
			*/
			//execlp(arg[0], arg[1], NULL);
			execvp(arg[0], arg);
			perror(arg[0]);
			_exit(1);
		}
		else
		{
			int out = open("/dev/null", O_RDWR | O_TRUNC);
			int in = open("/dev/null", O_RDWR | O_TRUNC);
			fcntl(out, F_SETFD, FD_CLOEXEC);
			fcntl(in, F_SETFD, FD_CLOEXEC);
			dup2(out, STDOUT_FILENO);
			dup2(in, STDIN_FILENO);
			execvp(arg[0], arg);
			perror(arg[0]);
			_exit(1);
		}
	} 
	// only executed by the parent
	dup2(std_out, STDOUT_FILENO);
	dup2(std_in, STDIN_FILENO);
	spawnPid = waitpid(spawnPid, &status, WNOHANG);

	return spawnPid;
}

///////////////////////////////////////////////////////////////////////////////
// constructs the argument list from the array of pointers to 
// null-terminated strings
// the syntax is: command [arg1 arg2 ...] [< in_file] [> out_file]
// input: the head of the linked list of commands
// output: stdout
void processBashCommands(struct node* cmd, int status)
{
	struct node* ref = cmd;
	char *argv[3];
	char cmdOpts[MAX_LEN] = "";
	char* cmdStr = malloc((strlen(cmd->val) + 1) * (sizeof(char)));
	strcpy(cmdStr, cmd->val);
	cmd = cmd->next;
	argv[0] = cmdStr;
	int flag = 0;

	// at this point, all this is being used for is to check the end
	// of the list of commands for &, <, >, and to create argv
	while(cmd != NULL)
	{
		flag = 1;
		// if the < or > are encountered, set a symbol to splice the string
		// It still needs to be read this way because the last char & is
		// being checked. 
		if (strcmp(cmd->val, "<") == 0)
		{
			strcat(cmdOpts, "#");
			cmd = cmd->next;
		}
		else if ((strcmp(cmd->val, ">") == 0))
		{
			strcat(cmdOpts, "#");
			cmd = cmd->next;
		}
		else
		{
			strcat(cmdOpts, cmd->val);
			strcat(cmdOpts, "\0");
			cmd = cmd->next;
		}
	}

	// check the last character before changing string
	if (cmdOpts[strlen(cmdOpts) - 1] == '&')
	{
		//printf("last char is &\n");
		cmdOpts[strlen(cmdOpts) - 1] = '\0';
		flag = 2;
	}
	//printf("%s %i\n", cmdOpts, strlen(cmdOpts));

	///*
	//printf("cmdOpts: %s\n", cmdOpts);
	// get rid of all the cmdOpts characters after the first # is found.
	for (int i = 0; i < strlen(cmdOpts); ++i)
	{
		if (cmdOpts[i] == '#')
		{
			// write over the remaining characters with nulls
			for (int j = i; j < strlen(cmdOpts); ++j)
			{
				cmdOpts[j] = '\0';
			}
		}
	}	
	//printf("cmdOpts: %s\n", cmdOpts);
	//*/

	// check if there are options or nah. 
	if (flag){ argv[1] = cmdOpts; argv[2] = NULL; }
	else { argv[1] = NULL; argv[2] = NULL; }
	
	// choose whether it is a foreground or background process
	if (flag == 2)
	{
		runBackground(argv, cmdOpts, ref);
	}
	else 
	{
		runForeground(argv, cmdOpts, ref, status);	
	}
	free(cmdStr);
}

///////////////////////////////////////////////////////////////////////////////
// check the kind of command to run. if it is 4, process differently
// 	1: exit 
// 	2: cd
// 	3: status
// 	4: all others
// input: a list of commands 
// ouput: the result of those commands
void peek_commands(struct node* cmds, int statusPtr) 
{
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
			_exit(0);
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
			/*// can comment out at the end
			temp = get_current_dir_name();
			printf("%s\n", temp);
			free(temp);
			*/		
		break;
		case 3:
			printf("exit status %d\n", statusPtr); 
		break;
		case 4:
			processBashCommands(head, statusPtr);		
		break;
		default:
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
int main() 
{
	char smallsh[8] = "smallsh";
	setenv(smallsh, "smallsh", 1);
	char **input = malloc(sizeof(char) * MAX_LEN + 1);
	struct node* commands = NULL;
	size_t len;
	int status = 0;
	// im not able to think of how to integrate this array into the program.
	// i need more time, im slow
	int bgProc[200];
	for (int i = 0; i < 200; ++i) { bgProc[i] = 0; }

	// Enter the smallsh command line
	while(1) 
	{
		printf(": ");
		// read the input
		if(getline(input, &len, stdin) < 1) {
			printf("Yea, no...\n");
			perror("getline");
		}
		commands = CreateCommandList(input);
		if (commands != NULL)
		{
			peek_commands(commands, status);
			destroyCommandList(commands);
		}
		fflush(stdin);
		fflush(stdout);
		strcpy(*input, "");
	}
	// somehow I thought i'd get to this
	/*
	do
	{
		wPid = waitpid(spawnPid, &childStatus, WNOHANG);
		execvp(arg[0], arg);
		perror(arg[0]);
		if (wPid < 0)
		{
			perror("waitpid");
			_exit(1);
		}
		if (WIFEXITED(childStatus))
		{
			printf("child exited, status=%d\n", WEXITSTATUS(childStatus));
		}
		else if (WIFSIGNALED(childStatus))
		{
			printf("child killed (signal %d)\n", WTERMSIG(childStatus));
		}
		else if (WIFSTOPPED(childStatus))
		{
			printf("child stopped (signal %d)\n", WSTOPSIG(childStatus));
    }
	} while(wPid >= 0); 
		//while (WIFEXITED(childStatus) && !WIFSIGNALED(childStatus));
	*/

	free(input);
	return 0;
}
