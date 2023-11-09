#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    pid_t pid;
    pid_t pid2;
    unsigned i;
    unsigned niterations = 100;
    pid = fork();
    
    if (pid == 0) {
        for (i = 0; i < niterations; ++i)
            printf("A = %d, ", i);
     } 
    else {
        pid2 = fork();

        if (pid2 == 0) {
            for (i = 0; i < niterations; ++i)
                printf("C = %d, ", i);
        }
        else {
            for (i = 0; i < niterations; ++i)
                printf("B = %d, ", i);
            
            printf("\nThis is the childprocess A and it has process ID: %d\n", pid);
            printf("This is the childprocess C and it has process ID: %d\n", pid2);
        }
    }
    printf("\n");
}
