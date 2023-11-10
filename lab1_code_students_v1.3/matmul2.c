/***************************************************************************
 *
 * Sequential version of Matrix-Matrix multiplication
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 

#define SIZE 1024
#define NUM_THREADS 1
static double a[SIZE][SIZE];
static double b[SIZE][SIZE];
static double c[SIZE][SIZE];

static void init_a(void){
    int i, j;
    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
            a[i][j] = (double)(i + j);
}

static void init_b(void){
    int i, j;
    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
            b[i][j] = (double)(i * j);
}
static void
init_matrix(void)
{
    pthread_t thread_a, thread_b;
    pthread_create(&thread_a, NULL, init_a, NULL);
    pthread_create(&thread_b, NULL, init_b, NULL);
    pthread_join(thread_a, NULL);
    pthread_join(thread_b, NULL);
}

static void thread_multi(int split){
    int i, j, k;
    int start = split * SIZE / NUM_THREADS;
    int end = (split + 1) * SIZE / NUM_THREADS;

    for (i = start; i < end; i++) {
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
    for (i = 0; i < NUM_THREADS; i++)
        pthread_create(&thread, NULL, thread_multi, (void*)i);
    for (i = 0; i < NUM_THREADS; i++)
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
