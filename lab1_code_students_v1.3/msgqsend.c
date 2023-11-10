#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define INT_MAX 50

//Created function to generate 50 random numbers and placing them in an array on the heap
//Returns a pointer to the array
int* generate_50_numbers(){
   int i;
   int *numbers = malloc(50 * sizeof(int));
   for (i = 0; i < 50; i++){
      numbers[i] = (rand() % (INT_MAX-1)) + 1;
   }
   return numbers;
};

//Changed the mtext to be mnumber and setting the size to ine integer since we only send one integer at a time

#define PERMS 0644
struct my_msgbuf {
   long mtype;
   int mnumbers;
};

int main(void) {

   struct my_msgbuf buf;
      int msqid;
      int len;
      key_t key;
   system("touch msgq.txt"); // create the file to use as a token when generating the key for message queue

   // Reciving the pointer ot the array of 50 random numbers
   int *numbers = generate_50_numbers();


   if ((key = ftok("msgq.txt", 'B')) == -1) {
      perror("ftok");
      exit(1);
   }

   if ((msqid = msgget(key, PERMS | IPC_CREAT)) == -1) {
      perror("msgget");
      exit(1);
   }

   printf("message queue: ready to send messages.\n");
   printf("Enter lines of text, ^D to quit:\n");
   buf.mtype = 1; /* we don't really care in this case */

   //Created a loop that loops through the array of 50 random numbers and sends them one by one
   for (int i = 0; i < 50; i++){
      //Sleeps the sending with 0.1 seconds to make it easier to see the numbers being sent
      usleep(100000);
      buf.mnumbers = numbers[i];
      printf("Sending %d\n", buf.mnumbers);
      len = sizeof(buf.mnumbers);
      if (msgsnd(msqid, &buf, len, 0) == -1) /* +1 for '\0' */
         perror("msgsnd");
   }

   // send the end signal
   buf.mnumbers = 0;
   len = sizeof(buf.mnumbers);
   printf("Sending end signal; %d\n", buf.mnumbers);
   if (msgsnd(msqid, &buf, len, 0) == -1) /* +1 for '\0' */
      perror("msgsnd");

   printf("message queue: done sending messages.\n");
   free(numbers);
   return 0;
}
