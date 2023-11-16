/***************************************************************************
 *
 * Sequential version of Matrix-Matrix multiplication
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 1024
#define NUM_THREADS 4
static double a[SIZE][SIZE];
static double b[SIZE][SIZE];
static double c[SIZE][SIZE];

//Created a functon to initialize the matrix a
static void init_a(void){
    int i, j;
    for (i = 0; i < SIZE; i++) //For each row
        for (j = 0; j < SIZE; j++) //For each column
            a[i][j] = (double)(i + j); //Set the value of the element
}

//Created a functon to initialize the matrix b
static void init_b(void){
    int i, j;
    for (i = 0; i < SIZE; i++) //For each row
        for (j = 0; j < SIZE; j++) //For each column
            b[i][j] = (double)(i * j); //Set the value of the element
}
static void
init_matrix(void)
{
    pthread_t thread_a, thread_b; //initialize two threads for a and b
    pthread_create(&thread_a, NULL, init_a, NULL); //assign the threads to the functions
    pthread_create(&thread_b, NULL, init_b, NULL); //assign the threads to the functions
    pthread_join(thread_a, NULL); //wait for the threads to finish
    pthread_join(thread_b, NULL); //wait for the threads to finish
}

static void thread_multi(int split){
    int i, j, k;
    int start = split * SIZE / NUM_THREADS; //split the matrix into NUM_THREADS parts
    int end = (split + 1) * SIZE / NUM_THREADS;  //split the matrix into NUM_THREADS parts

    for (i = start; i < end; i++) { //for each row iside of a the given span
        for (j = 0; j < SIZE; j++) {
            c[i][j] = 0.0;
            for (k = 0; k < SIZE; k++)
                c[i][j] = c[i][j] + a[i][k] * b[k][j];
        }
    }
}

static void
matmul_seq()
{
    pthread_t thread;
    int i;
    for (i = 0; i < NUM_THREADS; i++) //create NUM_THREADS threads
        pthread_create(&thread, NULL, thread_multi, (void*)i);
    for (i = 0; i < NUM_THREADS; i++) //wait for the threads to finish
        pthread_join(thread, NULL);
}


static void
print_matrix(void)
{
    int i, j;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++)
	        printf(" %7.2f", c[i][j]);
	    printf("\n");
    }
}

int
main(int argc, char **argv)
{
    init_matrix();
    matmul_seq();
    //print_matrix();
}
