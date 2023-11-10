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

static void
init_matrix(void)
{
    int i, j;

    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++) {
	        /* Simple initialization, which enables us to easy check
	         * the correct answer. Each element in c will have the same
	         * value as SIZE after the matmul operation.
	         */
	        a[i][j] = 1.0;
	        b[i][j] = 1.0;
        }
}

static void thread_multi(double row){
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
    for (i = 0; i < SIZE; i++)
        pthread_create(&thread, NULL, thread_multi, (void*)i);
    for (i = 0; i < SIZE; i++)
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
