#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    pid_t pid;
    pid_t pid2; //Adding a new PID
    unsigned i;
    unsigned niterations = 100;
    pid = fork(); //Starting the first child process

    if (pid == 0) { //cheacking if the process is a child process
        for (i = 0; i < niterations; ++i)
            printf("A = %d, ", i);
     }
    else {
        pid2 = fork(); //Starting the second child from the main process

        if (pid2 == 0) { //Checking for the second child process
            for (i = 0; i < niterations; ++i)
                printf("C = %d, ", i);
        }
        else { //If the process is not a child process then it is the main process and here we print the PID's of the child processes and also all B=0, B=1, B=2, ... B=99
            for (i = 0; i < niterations; ++i)
                printf("B = %d, ", i);

            printf("\nThis is the childprocess A and it has process ID: %d\n", pid); //Revealing the PID
            printf("This is the childprocess C and it has process ID: %d\n", pid2); //Revealing the PID
        }
    }
    printf("\n");
}
