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

// fills buffer_1 
void fill_buf1(char val) {
	pthread_mutex_lock(&mutex1);
	buffer_1[prod1_idx] = val;
	prod1_idx = prod1_idx + 1;
	buf1_count = buf1_count + 1;	
	// unblocks threads blocked on a condition variable
	pthread_cond_signal(&full1);
	pthread_mutex_unlock(&mutex1);
}

// fills buffer_2
void fill_buf2(char val) {
	//printf("b2: %c\n",val);
	pthread_mutex_lock(&mutex2);
	buffer_2[prod2_idx] = val;
	buf2_count = buf2_count + 1;	
	prod2_idx = prod2_idx + 1;
	pthread_cond_signal(&full2);
	pthread_mutex_unlock(&mutex2);
}

// fills buffer_3
void fill_buf3(char val) {
	//printf("b3\n%c\n", val);
	pthread_mutex_lock(&mutex3);
	buffer_3[prod3_idx] = val;
	buf3_count = buf3_count + 1;	
	prod3_idx = prod3_idx + 1;
	pthread_cond_signal(&full3);
	pthread_mutex_unlock(&mutex3);
}

// get the next item from buffer 1
// if the buffer is empty, a wait condition
// is placed on the thread.
// else it consumes the data in buffer_1
// returns the character at the cons1_idx of the 
// buffer
char get_buf1() {
	char ch;
	pthread_mutex_lock(&mutex1);
	//printf("checking buf1\n");
	while (buf1_count == 0) {
		pthread_cond_wait(&full1, &mutex1);
	}
	ch = buffer_1[cons1_idx];
	cons1_idx = cons1_idx + 1;
	buf1_count = buf1_count - 1;
	pthread_mutex_unlock(&mutex1);
	return ch;
}

char get_buf2() {
	pthread_mutex_lock(&mutex2);
	while (buf2_count == 0) {
		pthread_cond_wait(&full2, &mutex2);
	}	
	char ch = buffer_2[cons2_idx];
	cons2_idx = cons2_idx + 1;
	buf2_count--;
	pthread_mutex_unlock(&mutex2);
	return ch;
}

char get_buf3() {
	pthread_mutex_lock(&mutex3);
	while (buf3_count == 0) {
		pthread_cond_wait(&full3, &mutex3);
	}
	char ch = buffer_3[cons3_idx];
	cons3_idx = cons3_idx + 1;
	buf3_count--;
	pthread_mutex_unlock(&mutex3);
	return ch;
}

void* output(void* args) {
//void output() {
	
	if ((buf3_count % 10) == 0) {
		for (int i = 0; i <	buf3_count; ++i) {
			printf("%c", buffer_3[i]);		
			fflush(stdout);
		}
		printf("\n");
		fflush(stdout);
	}

	return NULL;
	
}

void* plus_plus(void* args) {
//void plus_plus() {
	char ch1;
	char ch2;
	char ch3;

	for (int i = 0; i < buf2_count; ++i) {
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
	return NULL;
}

// replaces all occurences of the newline character with 
// space. retrieves the character values with the
// get_buf1 function
void* space_replace(void* args) {
//void space_replace() {
	char ch;	
	pthread_mutex_unlock(&mutex1);
	//printf("space replace consuming buffer_1, buf1 count: %i\n", buf1_count);
	//for (int i = 0; i < buf1_count; ++i) {
		ch = get_buf1();
		if (ch == '\n') {
			ch = ' '; 
		}	
		fill_buf2(ch);	
	//}
	return NULL;
}

// consumes input and passes it to buffer_1
void* read_input(void* args) {
	size_t size;
	char** line = calloc(LINE_LEN, sizeof(char));
	char* str = NULL;

	for (int i = 0; i < LINE_CNT; ++i) {
			getline(line, &size, stdin);
		if (strncmp(*line, "STOP\n",5) == 0) {
			term_sym = 1;
		} else {
			str = calloc(strlen(*line), sizeof(char));
			strcpy(str, *line);
			for (int i = 0; i < strlen(*line); i++) {
				fill_buf1(str[i]);	
				//printf("buffer_1 count: %i\n", buf1_count);
			}
			free(str);
		}
	}

	return NULL;
}

int main(int argc, char* argv[]) {
	srand(time(0));
	pthread_t input_t; 
	pthread_t replace_t;
	pthread_t plus_t;
	pthread_t output_t;
	
	do {
	pthread_create(&input_t, NULL, read_input, NULL);
	pthread_create(&replace_t, NULL, space_replace, NULL);
	pthread_create(&plus_t, NULL, plus_plus, NULL);
	pthread_create(&output_t, NULL, output, NULL);
	} while (term_sym == 0);

	pthread_join(input_t, NULL);
	pthread_join(replace_t, NULL);
	pthread_join(plus_t, NULL);
	pthread_join(output_t, NULL);

	/*
	printf("buffer 1\n");
	for (int i = 0; i < strlen(buffer_1); ++i) {
		printf("%c", buffer_1[i]);
	}
	*/
	/*
	printf("buffer 2\n");
	for (int i = 0; i < strlen(buffer_2); ++i) {
		printf("%c", buffer_2[i]);
	}
	*/
	/*
	printf("buffer 3\n");
	for (int i = 0; i < strlen(buffer_3); ++i) {
		printf("%c", buffer_3[i]);
	}
	*/

	printf("buffer 3: %s %ld\n", buffer_3, strlen(buffer_3));

	return EXIT_SUCCESS;
}
