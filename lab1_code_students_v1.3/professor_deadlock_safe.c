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
#include <errno.h>

#define SHMSIZE 128
#define NUM_PROCESSES 5

struct chop_stick
{
    sem_t chopstick_sem[5];
};

struct chop_stick *chst = NULL;

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

    // Create semaphores for each chopstick
    for (i = 0; i < 5; i++)
    {
        sem_init(&chst->chopstick_sem[i], 1, 1); // Initialize with value 1
    }

    for (i = 0; i < NUM_PROCESSES; i++)
    {
        pids[i] = fork();

        // Check for fork error
        if (pids[i] == -1)
        {
            fprintf(stderr, "Failed to fork process %d\n", i);
            exit(EXIT_FAILURE);
        }

        if (pids[i] == 0)
        {
            // Child process
            while (1)
            {

                // Check for the interrupted flag inside the loop
                if (interrupted)
                {
                    break;
                }

                int left_stick, right_stick = 0;
                printf("Professor %d is thinking\n", i);
                // Didn't create a think function because there is two different
                // think times intervals in the code
                int wait_time = (rand() % 4000000) + 1000000;
                usleep(wait_time);

                printf("Professor %d is hungry\n", i);

                // Take both chopsticks if available
                left_stick = take_chopstick(i);
                right_stick = take_chopstick((i + 1) % NUM_PROCESSES);
                if (left_stick && right_stick)
                {
                    printf("Professor %d has taken both chopsticks\n", i);
                    printf("Professor %d is eating\n", i);
                    printf("Professor %d is done eating\n\n", eat(i));

                    // Put chopsticks back on the table
                    put_chopstick(i);
                    put_chopstick((i + 1) % NUM_PROCESSES);

                    
                }
                else
                {
                    // Here it tries to put both chopsticks back on the table,
                    // if it couldn't take both chopsticks
                    printf("Both chopsticks not available for %d\n", i);
                    put_chopstick(i);
                    put_chopstick((i + 1) % NUM_PROCESSES);
                    continue;
                }
            }

            // Cleanup shared memory
            for (i = 0; i < 5; i++)
            {
                sem_destroy(&chst->chopstick_sem[i]);
            }
            shmctl(shmid, IPC_RMID, NULL);
            fflush(stdout);
            exit(EXIT_SUCCESS);
        }
    }

    // Wait for all child processes to complete
    for (i = 0; i < NUM_PROCESSES; i++)
    {
        wait(NULL);
    }

    // Cleanup shared memory
    for (i = 0; i < 5; i++)
    {
        sem_destroy(&chst->chopstick_sem[i]);
    }
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
     int result = sem_trywait(&chst->chopstick_sem[n]);
    
    if (result == 0) {
        // Successfully acquired the chopstick
        return 1;
    } else if (result == -1 && errno == EAGAIN) {
        // Chopstick is not available
        return 0;
    } else {
        // Error occurred
        perror("Error in take_chopstick");
        return 0;
    }
}

int put_chopstick(int n)
{
     int result = sem_post(&chst->chopstick_sem[n]);
    
    if (result == 0) {
        // Successfully released the chopstick
        return 1;
    } else {
        // Error occurred
        perror("Error in put_chopstick");
        return 0;
    }
}

int eat(int i)
{
    int wait_time = (rand() % 5000000) + 5000000;
    usleep(wait_time);
    return i;
}
