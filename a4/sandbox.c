#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#define NUM_THREADS 3
#define COUNT_TO 10000000
#define SIZE 5
#define END_MARKER -1
void *helloWorld(void *args) 
{
	printf("hey\n");
	return NULL;
}

struct item 
{
	int year;
	char* name;
};
// using a structute to pass multiple arguments to a thread
/*
void* perform_work(void* argument) 
{
	struct item* curr_item = (struct item*) argument;
	printf("name = %s, year = %d\n", curr_item->name, curr_item->year);
	return NULL;
}
*/

// waiting for threads to end
/*
void* perform_work(void* argument)
{
	pthread_t pw_tid = pthread_self();
	int passed_in_value;
	// cast the void* to int* and then dereference the int* to get the 
	// value the pointer is pointing to
	passed_in_value = *((int *) argument);
	printf("the thread with argument %ld\n", passed_in_value);
	printf("the id of calling thread %i\n", pw_tid);
	return NULL;
}
*/

// testing for two threads equality
/*
void* perform_work(void* argument)
{
	pthread_t pw_tid = pthread_self();
	printf("the thread id %ld\n", pw_tid);
	return NULL;
}
*/


// counter with race condition
/*
void* perform_work(void* argument)
{
	for (int i = 0; i < COUNT_TO; ++i)
	{
		counter += 1;	
	}	
	return NULL;
}
*/

// initialize mutaual exclusion to prevent race conditions
/*
pthread_mutex_t counterMutex;

int counter = 0;


void* perform_work(void* argument) {
	for (int i = 0; i < COUNT_TO; i++) {
		pthread_mutex_lock(&counterMutex);
		// critical section starts
		counter += 1;
		// critical section ends
		// unlock the mutex
		pthread_mutex_unlock(&counterMutex);
	}	
	return NULL;
}
*/

// Conditional variables for synchronization mechanisms
// shared resource
/*
int buffer[SIZE];
// number of items in the buffer, shared resource
int count = 0;
//index where the producer will put the next item
int prod_idx = 0;
//index where the consumer will pick up the next item
int cons_idx = 0;
//how many items will be produced before the END_MARKER
int num_items = 0;
//init the mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//produces a random integer between [0, 1000] unless it is the last 
//item to be produced, in which case the value -1 is returned
int produce_item(int i) {
	int value;
	if (i == num_items) {
		value = END_MARKER;
	} else {
		value = rand() % 1000;
	}	
	return value;
}
//put the item in teh shared buffer
int put_item(int value) {
	buffer[prod_idx] = value;
	// increment the index where the next item will be put. roll over
	// to the start of the buffer if the item was placed in the last
	// slot in the buffer
	prod_idx = (prod_idx + 1) % SIZE;
	count++;
	return value;
}
//function that the producer thread will run. produce an item . put in 
//the buffer only when there is space in the buffer. if the buffer is 
//full, then wait until there is space in the buffer.
void* producer(void* args) {
	for (int i = 0; i < num_items + 1; i++) {
		// produce the item outside the critical section
		int value = produce_item(i);
		//lock the mutex before checking where there is apce in the buffer
		pthread_mutex_lock(&mutex);
		while (count == SIZE) {
			// buffer if full. give up the lock
			pthread_mutex_unlock(&mutex);
			// sleep before checking the buffer for space
			sleep(1);
			// lock the mutex before checking the buffer for space
			pthread_mutex_lock(&mutex);
		}
		put_item(value);
		// give up the lock
		ptHread_mutex_unlock(&mutex);
		// print messae outside the critical section
		printf("PROD %d\n", value);
}
	return NULL;
}
// get the next item from the shared buffer
int get_item() {
	int value = buffer[cons_idx];
	// increment the index from which the item will be picked up, rolling
	// over the the start of the buffer if curently at the end of the 
	// buffer
	cons_idx = (cons_idx + 1) % SIZE;
	count--;
	return value;
}
// function that the consumer thread will run. get an item from the 
// buffer if the buffer is not empty. if the buffer is empty then wait
// until there is data in the buffer
void* consumer(void* args) {
	int value = 0;
	// continue consuming until the END_MARKER is seen
	while (value != END_MARKER) {
		// lock the mutex before checking if the buffer has data
		pthread_mutex_lock(&mutex);
		while (count == 0) {
			// buffer is empty. give up the mutex
			pthread_mutex_unlock(&mutex);
			// sleep for 1 second before checking the buffer
			sleep(1);
			// lock the mutex before checking if the buffer has data
			pthread_mutex_lock(&mutex);
		}
		value = get_item();
		// give up the mutex
		pthread_mutex_unlock(&mutex);
		printf("cons %d\n", value);
	}
	return NULL;
}
*/

// Producer-Consumer Using Condition variable
/*
int buffer[SIZE];
// number of items in the buffer, shared resource
int count = 0;
// index where the profucer will put the next item
int prod_idx = 0;
// index where the consumer will pick up the next item
int con_idx = 0;
// how many items will be produced before the END_MARKER
int num_items = 0;

// init the mitex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// init the condition variable
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

// produces a tandon integer between [0, 1000] unless it is the last 
// item to be produced in which case the value -1 is returned.
int produce_item(int i) {
	int value;
	if (i == num_items) {
		value = END_MARKER;
	} else {
		value = rand() % 1000;
	}
	return value;
}

// put an item in the shared buffer
int put_item(int value) {
	buffer[prod_idx] = value;
// increment the index where the next item will be put. roll over to the 
// start of the buffer if teh item was placed in the last slot in the buffer
	prod_idx = (prod_idx + 1) % SIZE;
	count++;
	return value;
}

// function that the producer thread will run. produce an item and put in the 
// buffer only if there is space in teh buffer. if the buffer is full, then
// wait until there is space in the buffer
void *producer(void *args) {
	for (int i = 0; i < num_items + 1; i++) {
		// produce the item outside the critical section
		int value = produce_item(i);
		// lock the mutex before checking where there is space in the buffer
		pthread_mutex_lock(&mutex);
		while (count == SIZE) {
			// buffer is full. wait for the consumer to signal that the buffer 
			// has space
			pthread_cond_wait(&empty, &mutex);
		}
		put_item(value);
		// signal to the consumer that the buffer is no longer empty
		pthread_cond_signal(&full);
		// unlock the mutex
		pthread_mutex_unlock(&mutex);
		printf("PROD %d\n", value);
	}

	return NULL;
}

// get the next item in the bufer
int get_item() {
	int value = buffer[con_idx];
	// incremt the index from which the item will be picked up, rolling over
	// to the start of the buffer if cuttently at the end of the buffer
	con_idx = (con_idx + 1) % SIZE;
	count--;
	return value;
}

// function that the consumer thread will run. get an item from the buffer
// if the buffer is not empty. if the buffer is empty then wait until there
// is data in the buffer.
void *consumer(void* args) {
	int value = 0;
	while (value != END_MARKER) {
		pthread_mutex_lock(&mutex);
		while (count == 0) {
			pthread_cond_wait(&full, &mutex);
		}
		value = get_item();
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&mutex);
		printf("CONS %d\n", value);
	}
	return NULL;
}
*/
// Size of the buffer
#define SIZE 15

// Number of items that will be produced before the END_MARKER. Note that this number is smaller than the size of the buffer. This means that we can model the buffer as unbounded
#define NUM_ITEMS 12

// Special marker used to indicate end of the producer data
#define END_MARKER -1

// Buffer, shared resource
int buffer[SIZE];
// Number of items in the buffer, shared resource
int count = 0;
// Index where the producer will put the next item
int prod_idx = 0;
// Index where the consumer will pick up the next item
int con_idx = 0;

// Initialize the mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Initialize the condition variables
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

/*
Produces a random integer between [0, 1000] unless it is the last item to be produced in which case the value -1 is returned.
*/
int produce_item(int i){
  int value;
  if (i == NUM_ITEMS)
      value = END_MARKER;
  else
      value = rand() % 1000;
  return value;
}

/*
 Put an item in the shared buffer
*/
int put_item(int value)
{
    buffer[prod_idx] = value;
    // Increment the index where the next item will be put. Roll over to the start of the buffer if the item was placed in the last slot in the buffer
    prod_idx = (prod_idx + 1) % SIZE;
    count++;
    return value;
}

/*
 Function that the producer thread will run. Produce an item and put in the buffer only if there is space in the buffer. If the buffer is full, then wait until there is space in the buffer.
*/
void *producer(void *args)
{
    for (int i = 0; i < NUM_ITEMS + 1; i++)
    {
      // Produce the item outside the critical section
      int value = produce_item(i);
      // Lock the mutex before checking where there is space in the buffer
      pthread_mutex_lock(&mutex);
      while (count == SIZE)
        // Buffer is full. Wait for the consumer to signal that the buffer has space
        pthread_cond_wait(&empty, &mutex);
      put_item(value);
      // Signal to the consumer that the buffer is no longer empty
      pthread_cond_signal(&full);
      // Unlock the mutex
      pthread_mutex_unlock(&mutex);
      // Print message outside the critical section
      printf("PROD %d\n", value);
    }
    return NULL;
}

/*
 Get the next item from the buffer
*/
int get_item()
{
    int value = buffer[con_idx];
    // Increment the index from which the item will be picked up, rolling 
    // over to the start of the buffer if currently at the end of the buffer
    con_idx = (con_idx + 1) % SIZE;
    count--;
    return value;
}

/*
 Function that the consumer thread will run. Get  an item from the buffer if 
 the buffer is not empty. If the buffer is empty then wait until there is data in the buffer.
*/
void *consumer(void *args)
{
    int value = 0;
    // Continue consuming until the END_MARKER is seen    
    while (value != END_MARKER)
    {
      // Lock the mutex before checking if the buffer has data      
      pthread_mutex_lock(&mutex);
      while (count == 0)
        // Buffer is empty. Wait for the producer to signal that the buffer has data
        pthread_cond_wait(&full, &mutex);
      value = get_item();
      // Signal to the producer that the buffer has space
      pthread_cond_signal(&empty);
      // Unlock the mutex
      pthread_mutex_unlock(&mutex);
      // Print the message outside the critical section
      printf("CONS %d\n", value);
    }
    return NULL;
}

int main(int argc, char* argv[])
{
	// unbounded 
	srand(time(0));
    	pthread_t p, c;
    	// Create the producer thread
    	pthread_create(&p, NULL, producer, NULL);
    	// Now create the consumer thread
    	pthread_create(&c, NULL, consumer, NULL);
    	pthread_join(p, NULL);
    	pthread_join(c, NULL);
	
	// producer consumer using conditional variable
	/*
	if (argc != 2) {
		printf("usage: ./sandbox number of items\n");
		exit(1);
	}

	srand(time(0));
	num_items = atoi(argv[1]);
	pthread_t p, c;

	pthread_create(&p, NULL, producer, NULL);
	sleep(5);
	pthread_create(&c, NULL, consumer, NULL);
	pthread_join(p, NULL);
	pthread_join(c, NULL);
	*/
	
	
	//conditional variables for shared resources using a buffer,
	//and a producer and consumer
	/*
	if (argc != 2) {
		printf("usage: ./sandbox num_items\n");
		printf("provide number of items for the program to use\n");
		exit(1);
	}
	srand(time(0));
	num_items = atoi(argv[1]);
	pthread_t p, c;
	// create the producer thread
	pthread_create(&p, NULL, producer, NULL);
	// sleep to allow the producer to fill up the buffer
	sleep(5);
	// create the consumer thread
	pthread_create(&c, NULL, consumer, NULL);
	// wait for the threads to finish
	assert(pthread_join(p,NULL) == 0);
	assert(pthread_join(c,NULL) == 0);
	*/
	

	// init mutual exclusion to prevent race conditions
	/*
	pthread_mutex_init(&counterMutex, NULL);
	pthread_t threads[NUM_THREADS];	
	// start the threads
	for (int i = 0; i < NUM_THREADS; ++i) {
		pthread_create(&threads[i], NULL, perform_work, NULL);
	}
	// wait for the threads to terminate
	for (int i = 0; i < NUM_THREADS; i++) {
		assert(pthread_join(threads[i], NULL) == 0);
	}
	// destroy the mutex
	pthread_mutex_destroy(&counterMutex);
	printf("ecpected value of the counter: %d\n", COUNT_TO * NUM_THREADS);
	printf("actual value of the counter: %d\n", counter);
	exit(EXIT_SUCCESS);	
	*/
	
	// counter with race condition
	/*
	pthread_t threads[NUM_THREADS];
	// create NUM_THREADS threads
	for (int i = 0; i < NUM_THREADS; i++)
	{
		pthread_create(&threads[i], NULL, perform_work, NULL);
	}	
	// wait for threads to end
	for (int i = 0; i < NUM_THREADS; i++) {
		assert(pthread_join(threads[i], NULL) == 0);
	}
	printf("expected value of counter = %d\n", COUNT_TO * NUM_THREADS);
	printf("actual value of counter = %d\n", counter);
	exit(EXIT_SUCCESS);
	*/
	
	
	// Comparing threads for equality
	/*
	pthread_t t1;
	pthread_t t2;
	
	pthread_create(&t1, NULL, perform_work, NULL);
	assert(pthread_join(t1, NULL) == 0);
	pthread_create(&t2, NULL, perform_work, NULL);	
	assert(pthread_join(t2, NULL) == 0);
	
	printf("all threads completed successfully\n");
	exit(EXIT_SUCCESS);
	*/

	// Waiting for threads to end
	/*
	pthread_t threads[NUM_THREADS];
	int thread_args[NUM_THREADS];
	int result_code, index;	
	for (index = 0; index < NUM_THREADS; ++index)
	{
		// create all threads one by one
		thread_args[index] = index;
		printf("creating thread %d\n", index);
		result_code = pthread_create(&threads[index],
																	NULL,
																	perform_work,
																	(void *) &thread_args[index]);
		assert(0 == result_code);
	}	
	printf("all threads have compoleted successfully\n");
	exit(EXIT_SUCCESS);
	*/ 
		

	// Using a structure to pass multiple arguments to a thread
	/*
	struct item* an_item = calloc(1, sizeof(struct item));
	an_item->name = strdup("some person");
	an_item->year = 2012;

	pthread_t t;
	int result_code = pthread_create(&t, NULL, perform_work, (void*)an_item);
	result_code = pthread_join(t, NULL);
	printf("%i\n", result_code);
	free(an_item->name);
	free(an_item);
	exit(EXIT_SUCCESS);
	*/


	/* // example 1
	pthread_t tid;	
	
	// [0] &tid points to a variable into which the id of the new thread is written.
	// its type is above. and i spaltform dependent.. most likely an unsigned int
	//
	// [1] NULL points to a pthread_attr_t struct that contains option flags
	// 		it is null of no flags are to be passed
	//
	// [2] pointer function to the starting point of the new thread
	//
	// [3] this is the sole argument that is passed into [2]
	pthread_create(&tid, NULL, helloWorld, NULL);

	pthread_join(tid, NULL);
	*/
	
	return 0;
}
