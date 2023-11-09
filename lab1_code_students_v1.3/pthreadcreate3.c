#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

struct threadArgs {
	unsigned int id;
	unsigned int numThreads;
	unsigned int squaredId; //Added a new variable to the struct that holds the squared id, made ut unsigned sice a squared number cant be negative
};

void* child(void* params) {
	struct threadArgs *args = (struct threadArgs*) params;
	unsigned int childID = args->id;
	unsigned int numThreads = args->numThreads;
	args->squaredId = pow(args->id,2); //Added a line that sets the squaredId to the squared id
	printf("Greetings from child #%u of %u\n", childID, numThreads);
}

int main(int argc, char** argv) {
	pthread_t* children; // dynamic array of child threads
	struct threadArgs* args; // argument buffer
	unsigned int numThreads = 3;
	// get desired # of threads
	if (argc > 1)
		numThreads = atoi(argv[1]);
	children = malloc(numThreads * sizeof(pthread_t)); // allocate array of handles
	args = malloc(numThreads * sizeof(struct threadArgs)); // args vector
	for (unsigned int id = 0; id < numThreads; id++) {
		// create threads
		args[id].id = id;
		args[id].numThreads = numThreads;
		pthread_create(&(children[id]), // our handle for the child
			NULL, // attributes of the child
			child, // the function it should run
			(void*)&args[id]); // args to that function
	}
	printf("I am the parent (main) thread.\n");
	for (unsigned int id = 0; id < numThreads; id++) {
		pthread_join(children[id], NULL );
	}
	for (int i = 0; i < numThreads; i++){ //Added a for loop that loops through all the created structs and prints the squared id
		printf("The square of %u is %u\n", args[i].id, args[i].squaredId);
	}
	free(args); // deallocate args vector
	free(children); // deallocate array
	return 0;
}
