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
	buf1_count++;	
	prod1_idx++;
	pthread_cond_signal(&full1);
	pthread_mutex_unlock(&mutex1);
}

// fills buffer_2
void fill_buf2(char val) {
	pthread_mutex_lock(&mutex2);
	buffer_2[prod2_idx] = val;
	buf2_count++;	
	prod2_idx++;
	pthread_cond_signal(&full2);
	pthread_mutex_unlock(&mutex2);
}

// fills buffer_3
void fill_buf3(char val) {
	pthread_mutex_lock(&mutex3);
	buffer_3[prod3_idx] = val;
	buf3_count++;	
	prod3_idx++;
	pthread_cond_signal(&full3);
	pthread_mutex_unlock(&mutex3);
}

void* output(void* args) {
	int i = 0;
	while (buf3_count >= 0) {
		if ((i % 5) == 0) { printf("\n"); }
		printf("%c", buffer_3[i]);
		i++;
		buf3_count--;
	}
	return NULL;
}

void* plus_plus(void* args) {
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
	return NULL;
}

// turn into thread 2
void* space_replace(void* args) {
	while (buf1_count >= 0) {
		printf("%c",buffer_1[prod1_idx - 1]);
		if (buffer_1[prod1_idx - 1] == '\n') {
			buffer_1[prod1_idx - 1] = ' ';			
		}
		fill_buf2(buffer_1[prod1_idx - 1]);
		prod1_idx = prod1_idx - 1;
		buf1_count = buf1_count - 1;
	}
	
	return NULL;
}

// consumes input and passes it to buffer_1
void* read_input(void* args) {
	char ch;
	int i = 0;

	while (i < 10) {
		ch = get_char();
		fill_buf1(ch);	
		i++;
	}

	//space_replace();
	//plus_plus();
	//output();

	return NULL;
}

int main(int argc, char* argv[]) {
	
	srand(time(0));
	pthread_t input_t; 
	pthread_t replace_t;
	pthread_t plus_t;
	pthread_t output_t;
	
	pthread_create(&input_t, NULL, read_input, NULL);
	pthread_join(input_t, NULL);

	/*
	pthread_create(&replace_t, NULL, space_replace, NULL);
	pthread_create(&plus_t, NULL, plus_plus, NULL);
	pthread_create(&output_t, NULL, output, NULL);

	pthread_join(input_t, NULL);
	pthread_join(replace_t, NULL);
	pthread_join(plus_t, NULL);
	pthread_join(output_t, NULL);
	*/
	
	///*
	printf("buffer 1\n");
	for (int i = 0; i < strlen(buffer_1); ++i) {
		printf("%c", buffer_1[i]);
	}
	//printf("buffer 2: %s .\n", buffer_2);
	//printf("buffer 3: %s .\n", buffer_3);
	//*/

	return EXIT_SUCCESS;
}
