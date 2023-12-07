#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h> /* For O_* constants */
#define SHMSIZE 128
#define SHM_R 0400
#define SHM_W 0200

sem_t *mutex_sem, *empty_sem, *full_sem;

int main(int argc, char **argv)
{
	struct shm_struct {
		int buffer[10];
		int readIndex;
		int writeIndex;
		int count;
	};
	volatile struct shm_struct *shmp = NULL;
	char *addr = NULL;
	pid_t pid = -1;
	int var1 = 0, var2 = 0, shmid = -1;
	struct shmid_ds *shm_buf;

	srand(time(NULL));
	/* create semaphores */
	/* 0644 means read and write */
	empty_sem = sem_open("/sem_empty", O_CREAT, 0644, 10);
	full_sem = sem_open("/sem_full", O_CREAT, 0644, 0);



	/* allocate a chunk of shared memory */
	shmid = shmget(IPC_PRIVATE, SHMSIZE, IPC_CREAT | SHM_R | SHM_W);
	shmp = (struct shm_struct *) shmat(shmid, addr, 0);
	shmp->count = 0;
	shmp->readIndex = 0;
	shmp->writeIndex = 0;
	pid = fork();
	if (pid != 0) {
		/* here's the parent, acting as producer */
		while (var1 < 100) {
			/* write to shmem */
			var1++;


			int wait_time = (rand() % 400000) + 100000;
			usleep(wait_time);

			sem_wait(empty_sem);

			shmp->buffer[shmp->writeIndex] = var1;
			shmp->writeIndex = (shmp->writeIndex + 1) % 10;
			shmp->count++;
			printf("Sending %d\n", var1); 

			sem_post(full_sem);
		}
		shmdt(addr);
		shmctl(shmid, IPC_RMID, shm_buf);
	} else {
		/* here's the child, acting as consumer */
		while (var2 < 100) {
			/* read from shmem */
			
			sem_wait(full_sem);

			var2 = shmp->buffer[shmp->readIndex];
			shmp->readIndex = (shmp->readIndex + 1) % 10;
			shmp->count--;

			int wait_time = (rand() % 1800000) + 200000;
			usleep(wait_time);

			printf("Received %d\n", var2);

			sem_post(empty_sem);
		}
		shmdt(addr);
		shmctl(shmid, IPC_RMID, shm_buf);
	}
	fflush(stdout);
	sem_close(empty_sem);
	sem_close(full_sem);
	sem_unlink("/sem_empty");
	sem_unlink("/sem_full");
}
