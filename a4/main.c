#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define SIZE 50000
#define COUNT 80

int prod_idx = 0;
int cons_idx = 0;
char buffer[COUNT];
int count = 0;

pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;

char get_input() {
	char str;
	scanf("%c", &str);
	return str;
}

void put_into_buf(char val) {
	pthread_mutex_lock(&mutex);
	buffer[prod_idx] = val;
	prod_idx = prod_idx + 1;
	pthread_cond_signal(&full);
	pthread_mutex_unlock(&mutex);
}

void* load_input(void* args) {
	for (int i = 0; i < COUNT; i++) {
		char ch = get_input();
		put_into_buf(ch);
		count++;
	}
	return NULL;
}

int main(int argc, char* argv[]) {
	
	srand(time(0));
	pthread_t input_t;
	
	pthread_create(&input_t, NULL, load_input, NULL);

	pthread_join(input_t, NULL);

	printf("%d\n", count);
	printf("%s\n", buffer);

	return 0;
}
