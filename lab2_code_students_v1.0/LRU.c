#include <stdio.h>
#include <stdlib.h>
#include <math.h>

FILE *fptr;
int length_of_array;


// Open a file in read mode
int* open_file(char* file){
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

//adding a new page to the array and removing the least used
int add_new_page(int page_index,int *pages, int no_pyhsical_pages){
    for (int i = no_pyhsical_pages; i > 0; i--){
        pages[i] = pages[i-1];
    }
    pages[0] = page_index;
}

//If a page is already in the "RAM" then move it to the most recent used
int move_priority(int standing_index,int *pages, int no_pyhsical_pages){
    int saving_page = pages[standing_index];
    for (int i = standing_index; i > 0; i--){
        pages[i] = pages[i-1];
    }
    pages[0] = saving_page;
}

int pages_lru(int* array_ptr, int no_pyhsical_pages, int pages_size){
    //Variables to be used as pointers and flags
    int pages[no_pyhsical_pages];
    int page_faults = 0;
    int pages_size_tmp = pages_size;
    int page_index;
    int hit = 0;
    int used_pages = 0;

    //"loading in adresses"
    for (int i = 0; i < length_of_array; i++){
        page_index = ceil(array_ptr[i]/pages_size_tmp)+1;
        hit = 0;
        
        //Cheking if the pages array "RAM" is full or if it needs to be filled
        if (used_pages < no_pyhsical_pages){
            for (int j = 0; j<no_pyhsical_pages;j++){
                if (pages[j] == page_index){
                    hit = 1;
                    break;
                }
            }
            //If not in the empty array add it to most recent used
            if (hit == 0){
                add_new_page(page_index,pages,no_pyhsical_pages);
                page_faults++;
                used_pages++;
            }
        }

        //If "RAM" is full then check if the page is in the array
        else{
        for (int j = 0; j<no_pyhsical_pages;j++){
            //If in "RAM" then move page to the most recent used
            if (pages[j] == page_index){
                move_priority(j,pages,no_pyhsical_pages);
                hit = 1;
                break;
            }
        }
        //If not in "RAM" then delete the least used and add the new page to the most recent used
        if (hit == 0){
            page_faults++;
            add_new_page(page_index,pages,no_pyhsical_pages);
        }
        }
    }
    //Return number of page faults
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
    int faults = pages_lru(array_ptr,no_pyhsical_pages,pages_size);

    //Printing the result
    printf("Result: %d page faults\n", faults);

    //Cleaning up
    close_file();
    free(array_ptr);

return 0;
}