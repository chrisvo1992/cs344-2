#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define SIZE 50000
#define COUNT 80

// buffers 1 and 2 used for all preprocessing.
// buffer 3 is the final consumer
char buffer_1[SIZE + 1];
char buffer_2[SIZE + 1];
char buffer_3[COUNT + 1];

int prod1_idx = 0;
int cons1_idx = 0;

int prod2_idx = 0;
int cons2_idx = 0;

// a segment to check for the stopping condition 
char check_arr[6];
int char_count = 0;
char check1[6] = " STOP ";
char check2[6] = "\nSTOP\n";

// if true, stops reading from stdin
int term_sym = 0;

// count keeps track of buffer_3
int count = 0;

// not greater than 2
int plus_counter = 0;
pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;

// reads one char, no checking, not needed
char get_char() {
	char str;
	scanf("%c", &str);
	return str;
}

// fills buffer_1 
void fill_check_arr(char val) {
	pthread_mutex_lock(&mutex);
	check_arr[char_count] = val;
	char_count = char_count + 1;
	pthread_cond_signal(&full);
	pthread_mutex_unlock(&mutex);
}

// fills buffer_1 
void fill_buf1(char val) {
	pthread_mutex_lock(&mutex);
	buffer_1[prod1_idx] = val;
	prod1_idx = prod1_idx + 1;
	pthread_cond_signal(&full);
	pthread_mutex_unlock(&mutex);
}

// consumes input and passes it to buffer_1
void* read_input(void* args) {
	char ch;

	for (int i = 0; i < SIZE; i++) {
		ch = get_char();
		if (ch == '\n') {
			ch = ' ';
		}
		fill_check_arr(ch);
		char_count++;
		if (char_count == 6) {
			printf("check_arr: %s", check_arr);
			if (strcmp(check1, check_arr) == 0) {
				printf("found check1, stopping\n");
			}
			if (strcmp(check2, check_arr) == 0 ) {
				printf("found check2, stopping\n");	
			}
			char_count = 0;	
		}
	}

	return NULL;
}

int main(int argc, char* argv[]) {
	
	srand(time(0));
	pthread_t input_t;
	
	pthread_create(&input_t, NULL, read_input, NULL);

	pthread_join(input_t, NULL);
	return 0;
}
