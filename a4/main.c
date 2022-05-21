#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <unistd.h>
#include <errno.h>

#define MAX_INPUT  1000
#define MAX_OUTPUT  80
//create global buffers
char buffer1[MAX_INPUT]; //will hold input from stdin
char buffer2[MAX_INPUT];
char buffer3[MAX_INPUT];
char output[MAX_OUTPUT];

//when user enters STOP\n, stop will = 0 and program will end
int stop = 1; //
int buff1 = 0;
int buff2 = 0;
int buff3 = 0;

//create mutexes
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;

//create conditions to signal threads
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty3 = PTHREAD_COND_INITIALIZER;

int isEmpty(char buffer[]){
    if (buffer[0] == '\0'){
        return 1;
    }
    return 0;
}

void* getInput(){
    while (stop!= 0){
        pthread_mutex_lock(&mutex1);
        int len = strlen(buffer1);
        while (len != 0){
            // Buffer1 is full. Wait for the consumer to signal that buffer1 has space
            pthread_cond_wait(&empty1, &mutex1);
        }
        printf("1 start here\n");
        fgets(buffer1, MAX_INPUT, stdin);

        if(strcmp(buffer1, "STOP\n")== 0){
            stop = 0;
        }
        printf("1 buffer = %s\n", buffer1);
        pthread_cond_signal(&full_1); // signal to separateLines() that buffer1 is full
        pthread_mutex_unlock(&mutex1);


    }
    return NULL;
}
void* separateLines(){
    while(stop != 0){
        pthread_mutex_lock(&mutex1);
        int len = strlen(buffer1);
        while (len == 0){
            pthread_cond_wait(&full_1, &mutex1);
        }
        printf("test2\n");
        if(buffer1[len-1] == '\n'){
            buffer1[len-1] = ' ';
        }
        pthread_mutex_lock(&mutex2);

        strcpy(buffer2, buffer1);
        pthread_mutex_unlock(&mutex1);
        printf("buffer2 = %s\n",buffer2);

        pthread_cond_signal(&full_2); // signal plusSign() that buffer2 is filled
        pthread_mutex_unlock(&mutex2);

        //empty buffer1
        strcpy(buffer1, "");
        //pthread_cond_signal(&empty1);
        //pthread_mutex_unlock(&mutex1);
    }

    return NULL;
}

void* plusSign(){
    char copy[MAX_INPUT];
    int index1= 0;
    int index2= 0;
    int len;
    while (stop != 0){
         pthread_mutex_lock(&mutex2);
         len = strlen(buffer2) +1; // make len == 1 to continue while loop

         while(len == 1){
            pthread_cond_wait(&full_2, &mutex2);
            len = strlen(buffer2);

         }
         //int test = isEmpty(buffer2);
         //printf("test = %d\n", test);
         printf("test 3\n");
         strcpy(copy, buffer2);
         strcpy(buffer2, "");
         pthread_mutex_unlock(&mutex2);


         pthread_mutex_lock(&mutex3);

        int i, j;
        for (i=0, j=0; i<=1000; i++, j++){
            if (copy[i] == '+' && copy[i+1] == '+'){
                buffer3[j] = '^';
                i++;
            }
            else{
                buffer3[j] = copy[i];
            }
        }


         printf("buffer3 = %s\n", buffer3);
         pthread_mutex_unlock(&mutex3);

    }
    return NULL;
}
void* outputLine(){

}
int main(int argc, char *argv[])
{
    //create threads
    pthread_t input_t, line_separator_t, plus_sign_t, output_t;

    pthread_create(&input_t, NULL, getInput, NULL);
    pthread_create(&line_separator_t, NULL, separateLines, NULL);
    pthread_create(&plus_sign_t, NULL, plusSign, NULL);
    //pthread_create(&output_t, NULL, outputLine, NULL);

    //wait for threads to finish
    pthread_join(input_t, NULL);
    pthread_join(line_separator_t, NULL);
    pthread_join(plus_sign_t, NULL);
    //pthread_join(output_t, NULL);

    return 0;
}
