#include <stdio.h>
#include <stdlib.h>

struct chopStick{
    int id;
    int inUse;
};

struct professorsproblem{
    int id;
    int numberOfChopSticks;
};

void* professor(void* params){
    struct professorsproblem *args = (struct professorsproblem*) params;
    unsigned int professorID = args->id;
    unsigned int numberOfChopSticks = args->numberOfChopSticks;
    printf("Greetings from professor #%u of %u\n", professorID, numberOfChopSticks);
    free(args);
}

int main()
{
    professor(1);
    return 0;
}
