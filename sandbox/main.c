#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	//printf("PID is %d\n", getpid());
	//printf("parent PID is %d\n", getpid());
	
	// parent and child processes
	/*
	pid_t spawnpid = -5;
	int intVal = 10;
	spawnpid = fork(); // will be 0 if successful	
	switch (spawnpid) {
		case -1:
			perror("fork() failed");
			exit(1);	
			break;
		case 0:
			intVal = intVal + 1;
			printf("the child, intVal, is = %d\n", intVal);
			break;
		default:
			intVal = intVal - 1;
			printf("the parent is = %d\n", intVal);
			break;
	}
	printf("executed by both parent and child\n");
	// child processes inherit the following from the parent:
	// - the list of open file descriptors
	// - current working directory
	// - process group id
	// - env variables
	// - resource limits
	// - settings about how to handle signals
	//
	*/
	
	// monitoring child processes
	/*
	pid_t spawnpid = -5;
	int childStatus;
	int childPid;	
	spawnpid = fork();
	switch (spawnpid) {
		case -1:
			perror("fork() faild.\n");
			exit(1);
		case 0:
			printf("the child pid = %d\n", getpid());
			break;
		default:
			printf("the parent pid = %d\n", getpid());
			childPid = wait(&childStatus);
			printf("parent is done waiting as the child pid(%d) exited\n", childPid);
			break;
	}
	printf("The process with pid %d is returning from main\n", getpid());
	// limitations of wait:
	// - does not support waiting of a specific child
	// - no possibility of dong a non-blocking wait (blocks like a pro)
	//
	// waitpid addresses these issues
	*/
	
	// waitpid 
	/*
	int childStatus;
	printf("parent process's pid = %s\n", getpid());
	pid_t firstChild = fork();
	if (firstChild == -1) {
					perror("fork() falied\n");
					exit(1);
	} else if (firstChild == 0) {
		printf("first child's pid = %d\n", getpid());
		speep(10);
	} else {
		pid_t secondChild = fork();
		if (secondChild == -1) {
			perror("fork() failed\n");
			exit(1);
		} else if (secondChild == 0) {
			printf("second child's pid = %d\n", getpid());
			sleep(10);
		} else {
			pid_t childPid = waitpid(secondChild, &childStatus, 0);
			printf("the parent is done waiting. the pid of child that temrninated is\ 
				%d\n", childPid);
		}
	}
	printf("The process with pid %d is returning from main\n", getpid());
	*/
	
	// non blocking wait using wnohang
	/*
	int childStatus;
	printf("parent process's pid = %d\n", getpid());
	pid_t childPid = fork();
	if (childPid == -1) {
					perror("fork() falied\n");
					exit(1);
	} else if (childPid == 0) {
		printf("first child's pid = %d\n", getpid());
		sleep(10);
	} else {
		printf("childs pid = %d\n", childPid);
		childPid = waitpid(childPid, &childStatus, WNOHANG);
		printf("in the parent process waitpid returned value %d\n", childPid);
	}
	printf("The process with pid %d is returning from main\n", getpid());
	*/
	//

	// MACRO
	// WIFEXITED(wstatus):
	// 	= returns true if the child was terminated normally.
	// WEXITSTATUS(wstatus):
	// 	= if WIEXITED returned true, WEXITSTATUS will return the status value the 
	// 		child passed to exit().	
	// WIFSIGNALED(wstatus):
	// 	= returns true if the child was terminated abnormally. 
	// WTERMMSG(wstatus):
	// 	= if WIFSIGNALED returned true, WTERMMSG will return the signal number that
	// 		caused the child to terminate. if WIFEXITED returned true, WTERMSIGN may
	// 		return garbage.
	//
	//  Examples
	/*
	int childStatus;
	pid_t childPid = fork();

	if (childPid == -1) {
			perror("fork() failed\n");
			exit(1);
	} else if (childPid == 0) {
			sleep(10);
	} else {
			printf("childs pid = %d\n", childPid);
			childPid = waitpid(childPid, &childStatus, 0);
			printf("waitpid returned value %d\n", childPid);
			if (WIFEXITED(childStatus)){
					printf("child %d exited normally with status %d\n", 
							childPid, WEXITSTATUS(childStatus));
			} else {
				printf("child %d exited abnormally due to signal %d\n", childPid,
												WTERMSIG(childStatus));
			}
	}
	*/
	// Process API:
	// 
	// int execv(const char* pathname, const char* arg[]);
	/*
	char *newargv[] = { "/bin/ls", "-al", NULL };
	execv(newargv[0], newargv);
	perror("execv");
	exit(EXIT_FAILURE);
	*/
	//	
	// using exec() with fork()
	/*	
	char *newargv[] = { "/bin/ls", "-al", NULL };
	int childStatus;

	pid_t spawnPid = fork();

	switch(spawnPid) {
		case -1:
			perror("fork()\n");
			exit(1);
			break;	
		case 0:
			printf("child(%d) running ls command\n", getpid());
			execv(newargv[0], newargv);
			perror("execve");
			exit(2);
			break;
		default:
			spawnPid = waitpid(spawnPid, &childStatus, 0);
			printf("parent(%d): child(%d) terminated. exiting\n", getpid(),
				spawnPid);
			exit(0);
			break;
	}
	*/
	// using execl
	// parent only
	/*
	execl("/bin/ls", "/bin/ls", "-al", NULL);	
	perror("execl");
	exit(EXIT_FAILURE);
	*/
	// parent make child. child runs ls -al
	/*
	int childStatus;

	pid_t spawnPid = fork();

	switch(spawnPid) {
		case -1:
			perror("fork() failed\n");
			exit(1);
		break;
		case 0:
			printf("child(%d) running l command\n", getpid());
			execl("/bin/ls", "/bin/ls", "-al", NULL);
			perror("execl");
			exit(2);
			break;
		default:
			spawnPid = waitpid(spawnPid, &childStatus, 0);
			printf("parent(%d): child(%d) terminated. exiting\n",
				getpid(), spawnPid);
			exit(0);
		break;	
	}
	*/
	// using execlp
	/*
	execlp("ls", "ls", "-al", NULL);
	perror("execlp\n");
	exit(EXIT_FAILURE);
	*/
	// 
	// Exercise
	// prompt user for their name
	// use execv to call print_name.c
	// print_name.c prints a message that greets the user with the name 
	// 	that was specified as the first argument to the main function.
	///*
	char name[20] = "";
	printf("Enter a name: ");
	scanf("%s", name);
	char *newargv[] = { "print_name", name, NULL };
	execv(newargv[0], newargv);
	perror("execv");
	exit(EXIT_FAILURE);
	//*/
	return 0;
}
