/***************************************************************************
 *
 * Sequential version of Matrix-Matrix multiplication
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 1024

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

static void thread_multi(double row){ //the function takes in a row and computes the multiplication for every element on that row
    int j, k;
    for (j = 0; j < SIZE; j++) {
        c[(int)row][j] = 0.0;
        for (k = 0; k < SIZE; k++)
            c[(int)row][j] = c[(int)row][j] + a[(int)row][k] * b[k][j];
    }
}

static void
matmul_seq()
{
    pthread_t thread;
    int i;
    for (i = 0; i < SIZE; i++) //for each row creat a new thread
        pthread_create(&thread, NULL, thread_multi, (void*)i);
    for (i = 0; i < SIZE; i++) //wait for the threads to finish
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
