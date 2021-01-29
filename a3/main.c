#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
	int childStatus;
	char* varName = "MYVAR";

	setenv(varName, "foo", 1);

	printf("%s in parent is %s\n", varName, getenv(varName));
	pid_t spawnPid = fork();
	switch(spawnPid){
		case -1:
			perror("fork() failed");
			exit(1);
			break;
		case 0:
			printf("%s in child is %s\n", varName, getenv(varName));
			setenv(varName, "bar", 1);
			printf("%s in child has been updated to %s\n", varName, getenv(varName));
			break;
		default:
			spawnPid = waitpid(spawnPid, &childStatus, 0);
			printf("%s in parent is still %s\n", varName, getenv(varName));
			break;
	}
	return 0;	
}
