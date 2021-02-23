#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <assert.h>

#define LINE_LEN 1000
#define LINE_CNT 50
#define SIZE LINE_LEN * LINE_CNT 
#define COUNT 80

/*
 mutex4 acts as the boundary that the characters will 'stay within'
	
*/
int alvdbg = 0;
// buffers 1 and 2 used for all preprocessing.
// buffer 3 is the final consumer
char buffer_1[SIZE];
char buffer_2[SIZE];
char buffer_3[SIZE];

// count keeps track of buffer count
int buf1_count = 0;
int buf2_count = 0;
int buf3_count = 0;
int buf4_count = 0;

pthread_mutex_t  mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full1 = PTHREAD_COND_INITIALIZER;

pthread_mutex_t  mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full2 = PTHREAD_COND_INITIALIZER;

pthread_mutex_t  mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full3 = PTHREAD_COND_INITIALIZER;

pthread_mutex_t  mutex4 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full4 = PTHREAD_COND_INITIALIZER;

void* output(void* args) {
	int term_sym = 0;
	int last_line = 0;
	char output[81] = {0};
	char* line = calloc(LINE_LEN, sizeof(char));
	char* print_buffer = calloc(SIZE, sizeof(char));
		
	while (term_sym == 0) {
		pthread_mutex_lock(&mutex3);	

		while (buf3_count == 0 ) {
			pthread_cond_wait(&full3, &mutex3);
		}
		strcpy(line, buffer_3);
		strcat(print_buffer, buffer_3);

		buf3_count--;

		pthread_mutex_unlock(&mutex3);

		if (strcmp(line, "STOP\n") == 0) {	
			term_sym = 1;
			if (alvdbg) printf("at output\n");
		} else {

			int cur_line_count = (strlen(print_buffer))/80;

			for (int line = last_line; line < cur_line_count; line++) {
				memset(output, 0, 81); // clear the output line
				strncpy(output, print_buffer + (line*80), 80);

				printf("%s\n", output);

				fflush(stdout);
			}

			last_line = cur_line_count;
			
		}
		pthread_mutex_lock(&mutex4);
	 	buf4_count--;
		pthread_cond_signal(&full4);
		pthread_mutex_unlock(&mutex4);
	}

	if (alvdbg) printf("output exit\n");

	return NULL;
}

// requires len(replacement_term) <= len(term)
char* replaceTerm(const char *line, const char *term, const char *replacement_term)
{
	// fail if we have any null args
	assert(line);
	assert(term);
	assert(replacement_term);
	
    // note this depends on the target line len <= source line length
    char *buf = calloc(strlen(line)+1, sizeof(char));
    const char *p = NULL;
    const char *rest = line;
    // while we are still finding term 
    while((p = strstr(rest, term)))
    {
        strncat(buf, rest, (size_t)(p - rest));
        strcat(buf, replacement_term);
        rest = p + strlen(term);
    }
   // no more term, so just tack on what's left
    strcat(buf, rest);
    return buf;
}

void* plus_plus(void* args) {
	int term_sym = 0;
	char* str = calloc(LINE_LEN, sizeof(char));
	char* new_str;

	while (term_sym == 0 ) {

		pthread_mutex_lock(&mutex2);

		while (buf2_count == 0) {
			pthread_cond_wait(&full2, &mutex2);
		}
		strcpy(str, buffer_2);

		buf2_count--;

		pthread_mutex_unlock(&mutex2);

		if (strcmp(str, "STOP\n") == 0) {	
			term_sym = 1;
			if (alvdbg) printf("at plus\n");
			new_str = str;
		}	
		else { 
			new_str = replaceTerm(str, "++", "^");
		}

		pthread_mutex_lock(&mutex3);
	
		strcpy(buffer_3, new_str);
		buf3_count++;	

		pthread_cond_signal(&full3);
		pthread_mutex_unlock(&mutex3);
 	}

	if (alvdbg) printf("plus exit\n");

	return NULL;
}

// retrieving all values from buffer 1, therefore I need to 
// place a lock on buffer
// replaces all occurences of the newline character with 
// space. retrieves the character values with the
// get_buf1 function
void* space_replace(void* args) {
	char* str = calloc(LINE_LEN, sizeof(char));
	int term_sym = 0;	

	while (term_sym == 0) {

		pthread_mutex_lock(&mutex1);

		while (buf1_count == 0) {
			pthread_cond_wait(&full1, &mutex1);
		}

		strcpy(str, buffer_1);
		buf1_count--;

		pthread_mutex_unlock(&mutex1);
		
		if (strcmp(str, "STOP\n") == 0) {	
			term_sym = 1;
			if (alvdbg) printf("at space\n");
		}	else { 
			int len = strlen(str);
			if (str[len - 1] == '\n') {	
				str[len - 1] = ' ';
			}
		}

		pthread_mutex_lock(&mutex2);
			
		strcpy(buffer_2, str);
		
		buf2_count++;

		pthread_cond_signal(&full2);
		pthread_mutex_unlock(&mutex2);
	}

	if (alvdbg) printf("space end\n");

	return NULL;
}

// consumes input and passes it to buffer_1
void* read_input(void* args) {
	char* line = calloc(LINE_LEN, sizeof(char));
	int term_sym = 0;

	while (term_sym == 0) {
		fgets(line, LINE_LEN, stdin);

		pthread_mutex_lock(&mutex1);
	
		strcpy(buffer_1, line);	

		buf1_count++;

		pthread_cond_signal(&full1);
		pthread_mutex_unlock(&mutex1);

		pthread_mutex_lock(&mutex4);
		while (buf4_count == 0) {
			pthread_cond_wait(&full4, &mutex4);
		}
		buf4_count++;
		pthread_mutex_unlock(&mutex4);

		if (strcmp(line, "STOP\n") == 0) {	
			term_sym = 1;
		} 
	}

	if (alvdbg) printf("read end\n");

	return NULL;
}

int main(int argc, char* argv[]) {
	// yes, this happened 
	/*
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
	*/

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

	//close(fd1);
	//close(fd2);

	return EXIT_SUCCESS;
}
