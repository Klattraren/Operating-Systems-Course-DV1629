#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int* generate_50_numbers(){
   int i;
   int *numbers = malloc(50 * sizeof(int));
   for (i = 0; i < 50; i++){
      numbers[i] = (rand() % 49) + 1;
   }
   return numbers;
};

int main(void){
    printf("Generating 50 random numbers...\n");
    int *numbers = generate_50_numbers();
    printf("Done generating 50 random numbers.\n");

   int i;
   for (i = 0; i < 50; i++)
      printf("%d\n", numbers[i]);

    free(numbers);
    return 0;
}