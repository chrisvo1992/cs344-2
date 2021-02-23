#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>


#define LINE_LEN 1000
#define LINE_CNT 50
#define SIZE LINE_LEN * LINE_CNT 
#define COUNT 80

// buffers 1 and 2 used for all preprocessing.
// buffer 3 is the final consumer
char buffer_1[SIZE];
char buffer_2[SIZE];
char buffer_3[SIZE];

int prod1_idx = 0;
int cons1_idx = 0;

int prod2_idx = 0;
int cons2_idx = 0;

int prod3_idx = 0;
int cons3_idx = 0;

// if true, stops reading from stdin
int term_sym = 0;

// count keeps track of buffer count
int buf1_count = 0;
int buf2_count = 0;
int buf3_count = 0;

// never greater than 2
int plus_counter = 0;

pthread_mutex_t  mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full1 = PTHREAD_COND_INITIALIZER;

pthread_mutex_t  mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full2 = PTHREAD_COND_INITIALIZER;

pthread_mutex_t  mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full3 = PTHREAD_COND_INITIALIZER;

// reads one char, no checking, not needed
char get_char() {
	char str;
	scanf("%c", &str);
	return str;
}

void fill_buf1(char val) {
	buffer_1[prod1_idx] = val;
	prod1_idx = prod1_idx + 1;
	buf1_count = buf1_count + 1;	
}

char get_buf1() {
	char ch = buffer_1[cons1_idx];
	cons1_idx = cons1_idx + 1;
	buf1_count = buf1_count - 1;
	return ch;
}

void fill_buf2(char val) {
	buffer_2[prod2_idx] = val;
	prod2_idx = prod2_idx + 1;
	buf2_count = buf2_count + 1;	
}

char get_buf2() {
	char ch = buffer_2[cons2_idx];
	cons2_idx = cons2_idx + 1;
	buf2_count = buf2_count - 1;
	return ch;
}

void fill_buf3(char val) {
	buffer_3[prod3_idx] = val;
	prod3_idx = prod3_idx + 1;
	buf3_count = buf3_count + 1;	
}

char get_buf3() {
	char ch = buffer_3[cons3_idx];
	cons3_idx = cons3_idx + 1;
	buf3_count = buf3_count - 1;
	return ch;
}

void* output(void* args) {
	char str[SIZE] = "";
	int i = 0;

	while (term_sym == 0) {
		pthread_mutex_lock(&mutex3);	
		///*
		while (buf3_count == 0 ) {
			pthread_cond_wait(&full3, &mutex3);
		}
		//*/
		//pthread_mutex_unlock(&mutex3);

// t1 -> b1 -> t2 -> b2 -> t3 -> b3 -> t4

		while (buf3_count >= 0) {
			//printf("b3: %i\n", buf3_count);
			//fflush(stdout);
			str[i] = get_buf3();
			//if ((i % 10) == 0) {printf("\n");fflush(stdout);}
			printf("%c", str[i]);fflush(stdout);
			i++;
		}
		pthread_mutex_unlock(&mutex3);
	}
	return NULL;
}

void* plus_plus(void* args) {
	char ch1;
	char ch2;
	char ch3;

/*
 		Youur get_ and fill_ functions should be 
		in their own lock/unlock blocks. for example 
		in space_replace it should be:

    lock mutex1
    while empty
        wait(full1, mutex1)
    get_buf1 stuff
    unlock mutex1
    lock mutex2
    fill_buff2
    signal full2
    unlock mutex2
 */

	while (term_sym == 0 ) {

		pthread_mutex_lock(&mutex2);

		///*
		while (buf2_count < 2) {
			pthread_cond_wait(&full2, &mutex2);
		}
		//*/
		pthread_mutex_unlock(&mutex2);

		pthread_mutex_lock(&mutex3);
	
		while (buf2_count >= 0) {
			//printf("b2: %i\n", buf2_count);
			//fflush(stdout);
			ch1 = get_buf2();
			if (ch1 == '+') {
				ch2 = get_buf2();
				if (ch2 == '+') {
					ch3 = '^';
					fill_buf3(ch3);
				}	else {
					fill_buf3(ch1);
					fill_buf3(ch2);
				}
			} else {
				fill_buf3(ch1);
			}
		}
		pthread_cond_signal(&full3);
		pthread_mutex_unlock(&mutex3);
 	}
	return NULL;
}

// retrieving all values from buffer 1, therefore I need to 
// place a lock on buffer
// replaces all occurences of the newline character with 
// space. retrieves the character values with the
// get_buf1 function
void* space_replace(void* args) {
	char ch;	
	//char str[LINE_LEN] = "";
	//int i,count;
/*
 		Youur get_ and fill_ functions should be 
		in their own lock/unlock blocks. for example 
		in space_replace it should be:

    lock mutex1
    while empty
        wait(full1, mutex1)
    get_buf1 stuff
    unlock mutex1
    lock mutex2
    fill_buff2
    signal full2
    unlock mutex2
 */

	while (term_sym == 0) {
		//i = 0;
		//count = 0;

		pthread_mutex_lock(&mutex1);
		///*
		while (buf1_count == 0 ) {
			pthread_cond_wait(&full1, &mutex1);
		}
		//*/
		pthread_mutex_unlock(&mutex1);
		
		pthread_mutex_lock(&mutex2);

		while (buf1_count >= 0) {
			//printf("b1: %i\n", buf1_count);
			//fflush(stdout);
			ch = get_buf1();	
			if (ch == '\n') {
				ch = ' '; 
			}
			//str[i] = ch;	
			//count++;
			fill_buf2(ch);	
		}

		pthread_cond_signal(&full2);
		pthread_mutex_unlock(&mutex2);
			
		/*
		for (int j = 0; j < count; j++) {
			fill_buf2(str[j]);
		}	
		*/
		/*
		pthread_cond_signal(&full2);
		pthread_mutex_unlock(&mutex2);
		*/
	}
	return NULL;
}

// consumes input and passes it to buffer_1
void* read_input(void* args) {
	char line[LINE_LEN] = "";
	char* str = NULL;
/*
 		Youur get_ and fill_ functions should be 
		in their own lock/unlock blocks. for example 
		in space_replace it should be:

    lock mutex1
    while empty
        wait(full1, mutex1)
    get_buf1 stuff
    unlock mutex1
    lock mutex2
    fill_buff2
    signal full2
    unlock mutex2
 */

	while (term_sym == 0) {
		fgets(line, LINE_LEN, stdin);

		pthread_mutex_lock(&mutex1);
	
		if (strcmp(line, "STOP\n") == 0) {	
			term_sym = 1;
		} else {
			str = calloc(strlen(line), sizeof(char));
			strcpy(str, line);
			for (int j = 0; j < strlen(line); j++) {
				fill_buf1(str[j]);	
			}
			free(str);
		}

		pthread_cond_signal(&full1);
		pthread_mutex_unlock(&mutex1);
	}
	return NULL;
}

int main(int argc, char* argv[]) {
	int fd1, fd2;

	if (argc > 3) {
		// ./main > out < in 
		if (strcmp(argv[1], ">") == 0) {
			fd1 = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC);
			fd2 = open(argv[4], O_RDONLY);
			dup2(fd1, 1);
			dup2(fd2, 0);	
		}	
		// ./main < in > out
		if (strcmp(argv[1], "<") == 0) {
			fd1 = open(argv[2], O_RDONLY);
			fd2 = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC);
			dup2(fd1, 0);
			dup2(fd2, 1);	
		}	
	}

	if (argc > 1 && argc < 4) {
		// ./main > out
		if (strcmp(argv[1], ">") == 0) {
			fd1 = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC);
			dup2(fd1, 1);
		}	
		// ./main < in
		if (strcmp(argv[1], "<") == 0) {
			fd1 = open(argv[2], O_RDONLY);
			dup2(fd1, 0);
		}	
	}

	pthread_t input_t; 
	pthread_t replace_t;
	pthread_t plus_t;
	pthread_t output_t;

	pthread_create(&input_t, NULL, read_input, NULL);
	pthread_create(&replace_t, NULL, space_replace, NULL);
	pthread_create(&plus_t, NULL, plus_plus, NULL);
	pthread_create(&output_t, NULL, output, NULL);

	pthread_join(input_t, NULL);
	pthread_join(replace_t, NULL);
	pthread_join(plus_t, NULL);
	pthread_join(output_t, NULL);

	close(fd1);
	close(fd2);
	/*
	printf("buffer 1: %s %ld\n", buffer_1, strlen(buffer_1));
	printf("buffer 2: %s %ld\n", buffer_2, strlen(buffer_2));
	printf("buffer 3: %s %ld\n", buffer_3, strlen(buffer_3));
	*/
	return EXIT_SUCCESS;
}
