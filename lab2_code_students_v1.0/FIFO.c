#include <stdio.h>
#include <stdlib.h>
#include <math.h>

FILE *fptr;
int length_of_array;


// Open a file in read mode
int* open_file(char* file){
    //fptr = fopen(file, "r");
    fptr = fopen(file, "r");
    if (fptr == NULL)
    {
        printf("Cannot open file \n");
    }
    printf("Reading memory trace from %s...",file);    
    //Counting how many "adresses we have"
    while(!feof(fptr)){
        char ch = fgetc(fptr);
        if(ch == '\n'){
            length_of_array++;
        }
    }

    //Loading all the "adresses" it an array that is allocated on the heap
    printf("Read %d memory references\n",length_of_array);
    int* number_array = malloc(length_of_array * sizeof(int));

    //Resetting the file pointer to the start of the file
    rewind(fptr);

    //Loading all the "adresses" into the allocated array
    for (int i = 0; i < length_of_array; i++) {
        fscanf(fptr, "%d", &number_array[i]);
    }
    
    //Returning the pointer
    return number_array;
}

// Close the file
void close_file(){
    fclose(fptr);
}

int last_change(int page_index,int *pages, int no_pyhsical_pages){
    static int index = 0;
    pages[index%no_pyhsical_pages] = page_index;
    index++;
}

int fifo(int* array_ptr, int no_pyhsical_pages, int pages_size){
    int pages[no_pyhsical_pages];
    int pages_size_tmp = pages_size;
    int page_index;
    int page_faults = 0;
    int hit = 0;
    int used_pages = 0;

    //Looping through the array
    for (int i = 0; i < length_of_array; i++){
        page_index = ceil(array_ptr[i]/pages_size_tmp)+1;
        hit = 0;

        //Fill it up
        if (used_pages < no_pyhsical_pages){
            for (int j = 0; j<no_pyhsical_pages;j++){
                if (pages[j] == page_index){
                    hit = 1;
                    break;
                }
            }
            if (hit == 0){
                last_change(page_index,pages,no_pyhsical_pages);
                page_faults++;
                used_pages++;
            }
        }
        //If full
        else{
            for (int j = 0; j<no_pyhsical_pages;j++){
                if (pages[j] == page_index){
                    hit = 1;
                    break;
                }
            }
            if (hit == 0){
                last_change(page_index,pages,no_pyhsical_pages);
                page_faults++;
            }
        }


    }
    return page_faults;
}



int main(int argc, char *argv[]){
    //Taking input from command line
    int no_pyhsical_pages = atoi(argv[1]);
    int pages_size = atoi(argv[2]);
    char* file = argv[3];

    //Information print
    printf("No physical pages = %d, page size = %d\n", no_pyhsical_pages, pages_size);

    //Opening and loading all the "adresses into an array"
    int* array_ptr = open_file(file);

    //Running the LRU algorithm
    int faults = fifo(array_ptr,no_pyhsical_pages,pages_size);

    //Printing the result
    printf("Result: %d page faults\n", faults);

    //Cleaning up
    close_file();
    free(array_ptr);

return 0;
}