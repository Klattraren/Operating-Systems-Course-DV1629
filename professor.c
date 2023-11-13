#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <wait.h>

#define SHMSIZE 128
#define NUM_PROCESSES 5

struct chop_stick 
{
    int buffer[5];
};

volatile struct chop_stick *chst = NULL; 

sem_t *mutex_sem;

// Pre-declare functions
int think(int i);
int take_chopstick(int n);
int put_chopstick(int n);
int eat(int i);

int interrupted = 0; // Variable to check if the program was interrupted

void handle_signal(int signal)
{
    interrupted = 1;
    printf("Interrupted\n");
    fflush(stdout);
}

int main(int argc, char **argv)
{
    signal(SIGINT, handle_signal);
    char *addr = NULL;
    pid_t pids[NUM_PROCESSES];
    int i;
    int shmid = -1;

    srand(time(NULL));

    /* allocate a chunk of shared memory */
    shmid = shmget(IPC_PRIVATE, SHMSIZE, IPC_CREAT | SHM_R | SHM_W);
    chst = (struct chop_stick *)shmat(shmid, addr, 0);
    // Create semaphores
    mutex_sem = sem_open("/mutex_sema", O_CREAT, O_RDWR, 1);

    for (i = 0; i < NUM_PROCESSES; i++) {
        pids[i] = fork();

        // Check for fork error
        if (pids[i] == -1) {
            fprintf(stderr, "Failed to fork process %d\n", i);
            exit(EXIT_FAILURE);
        }

        if (pids[i] == 0) {
            while (1) {                

                 // Check for the interrupted flag inside the loop
                if (interrupted) {
                    sem_post(mutex_sem);
                    break;
                }
                
                int left_stick, right_stick = 0;
                printf("Professor %d is thinking\n", i);
                // Didn't create a think function because there is two different
                // think times intervals in the code
                int wait_time = (rand() % 4000000) + 1000000;
                usleep(wait_time);

                printf("Professor %d is hungry\n", i);

                // Take left chopstick if available
                left_stick = take_chopstick((i % 5));
                if (left_stick) {
                    printf("Professor %d has taken left chopstick\n", i);
                    printf("Professor %d is thinking\n", i);
                    int wait_time = (rand() % 6000000) + 2000000;
                    usleep(wait_time);

                } else {
                    printf("Professor %d couldn't take left chopstick\n", i);
                    printf("chst_value: %d\n", i);
                }

                // Take right chopstick if available
                right_stick = take_chopstick((i + 1) % 5);
                if (right_stick) {
                    printf("Professor %d has taken right chopstick\n", i);
                    printf("chst_value: %d\n", i + 1);
                } else {
                    printf("Professor %d couldn't take right chopstick\n", i);
                    printf("chst_value: %d\n", i + 1);
                }

                
                if (left_stick && right_stick){
                    printf("Professor %d is eating\n", i);
                    // Execute eat function in printf to compact code
                    printf("Professor %d is done eating\n\n", eat(i));

                    // Put chopsticks back on the table
                    put_chopstick((i % 5));
                    put_chopstick((i + 1) % 5);

                    fflush(stdout);
                }
                // If professor couldn't take both chopsticks, 
                // continue to the next iteration and try to take the other chopstick.
                //Will cause deadlock...

            }

            // Cleanup shared memory
            sem_close(mutex_sem);
            sem_unlink("/mutex_sema");
            shmctl(shmid, IPC_RMID, NULL);
            exit(EXIT_SUCCESS);
        }

    }

    // Wait for all child processes to complete
    for (i = 0; i < NUM_PROCESSES; i++) {
        wait(NULL);
    }

    // Cleanup shared memory
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}

int think(int i)
{
    int wait_time = (rand() % 4000000) + 1000000;
    usleep(wait_time);
    return i;
}

int take_chopstick(int n)
{
    sem_wait(mutex_sem);
    printf("So far so good\n"  );
    if (chst->buffer[n] == 0) {
        chst->buffer[n] = 1;
        sem_post(mutex_sem);
        return 1;
    } else {
        sem_post(mutex_sem);
        return 0;
    }
}

int put_chopstick(int n)
{
    sem_wait(mutex_sem);
    if (chst->buffer[n] == 1){
        chst->buffer[n] = 0;
    }
    else {
        chst->buffer[n] = 1;
    }
    sem_post(mutex_sem);
    return 1;
}

int eat(int i)
{
    int wait_time = (rand() % 5000000) + 5000000;
    usleep(wait_time);
    return i;
}
