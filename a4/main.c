#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>

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

int prod3_idx = 0;
int cons3_idx = 0;


// a segment to check for the stopping condition 
int stop_counter = 0;
//char stop[] = "STOP";
char stop[5] = {'S','T','O','P','\0'};

// if true, stops reading from stdin
int term_sym = 0;

// count keeps track of buffer count
int buf1_count = 0;
int buf2_count = 0;
int buf3_count = 0;

// never greater than 2
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
void fill_buf1(char* val) {
	pthread_mutex_lock(&mutex);
	for (int i = 0; i < strlen(val); ++i) {
		buffer_1[prod1_idx] = val[i];
		buf1_count++;	
		prod1_idx++;
	}
	pthread_cond_signal(&full);
	pthread_mutex_unlock(&mutex);
}

// fills buffer_2
void fill_buf2(char val) {
	pthread_mutex_lock(&mutex);
	buffer_2[prod2_idx] = val;
	buf2_count++;	
	prod2_idx++;
	pthread_cond_signal(&full);
	pthread_mutex_unlock(&mutex);
}

// fills buffer_3
void fill_buf3(char val) {
	pthread_mutex_lock(&mutex);
	buffer_3[prod3_idx] = val;
	buf3_count++;	
	prod3_idx++;
	pthread_cond_signal(&full);
	pthread_mutex_unlock(&mutex);
}

void output() {
	int i = 0;
	while (buf3_count >= 0) {
		if ((i % 5) == 0) { printf("\n"); }
		printf("%c", buffer_3[i]);
		i++;
		buf3_count--;
	}
}

void plus_plus() {
	int flag = 0;
	//printf("%d, %d\n", buf2_count, prod2_idx);

	prod2_idx -= 2;
	while (buf2_count >= 0) {
		//printf("%c. \n", buffer_2[prod2_idx]);

		if (buffer_2[prod2_idx] == '+' && buffer_2[prod2_idx - 1] == '+') {
			flag = 1;
		}

		buf2_count -= 1;

		if (flag) {
			fill_buf3('^');
			prod2_idx -= 2;
			flag = 0;
		} else {
			fill_buf3(buffer_2[prod2_idx]);
			prod2_idx--;
		}
	}
}

// turn into thread 2
void space_replace() {
	while (buf1_count >= 0) {
		//printf("%c",buffer_1[prod1_idx - 1]);
		if (buffer_1[prod1_idx - 1] == '\n') {
			buffer_1[prod1_idx - 1] = ' ';			
		}
		fill_buf2(buffer_1[prod1_idx - 1]);
		prod1_idx = prod1_idx - 1;
		buf1_count = buf1_count - 1;
	}
}

// consumes input and passes it to buffer_1
void* read_input(void* args) {
	size_t size;
	char* line = (char*)calloc(SIZE, sizeof(char));

	while (term_sym == 0) {
		getline(&line, &size, stdin);
		if (strncmp(line, "STOP\n", 5) == 0) {
			term_sym = 1;
			//return NULL;	
		} else {
			fill_buf1(line);	
		}	
	}

	space_replace();
	plus_plus();
	output();


	return NULL;
}

int main(int argc, char* argv[]) {
	
	srand(time(0));
	pthread_t input_t;
	
	pthread_create(&input_t, NULL, read_input, NULL);

	pthread_join(input_t, NULL);

	printf("buffer 1\n");
	for (int i = 0; i < strlen(buffer_1); ++i) {
		printf("%c", buffer_1[i]);
	}
	printf("buffer 2: %s .\n", buffer_2);
	printf("buffer 3: %s .\n", buffer_3);

	return 0;
}
