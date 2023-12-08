#include <iostream>
#include "fs.h"
#include <cstring>

/*---------------------------------------
MADE BY
Tobias Mattsson, tomt21@student.bth.se, DVAMI21h
Samuel Nyberg, sany@student.bth.se, DVAMI21h
-----------------------------------------*/

// Help functions

/*Finds the a free block in the FAT system and returns the index for it,
if no free block is found it returns -1*/
int
FS::find_free_block()
{

    for(int i = 2; i < NO_BLOCKS; i++){
        if(fat[i] == FAT_FREE){
            return i;
        }
    }
    return -1;
}


//Cheking if the block is the root block
int
FS::is_root(int block){
    if (block == ROOT_BLOCK){
        return 0;
    }
    return 1;
}

/*This function separates the filname from the actual path by splitting on the last / character */
void
FS::separate_name_dir(std::string filepath,std::string *filename,std::string *pre_path){
    int seperator_index = filepath.find_last_of("/");
    std::string tmp_filename = filepath.substr(seperator_index+1);
    if (strcasecmp(tmp_filename.c_str(),"..") == 0){
        *filename = "..";
        *pre_path = filepath;
    }else{
        *filename = tmp_filename;
        *pre_path = filepath.substr(0,seperator_index+1);
    }

}

/*Function takes path as inparameter and returns the sub-directory block or root
that the path leeds to*/
int
FS::get_subdiretory_from_path(std::string org_path){
    int block_to_access = current_dir.block;
    int seperator_index = org_path.find_first_of("/");
    std::string option = org_path.substr(0,seperator_index);
    std::string path = org_path.substr(seperator_index+1);

    //Reading in the block we stand in to change path
    dir_entry file_array[DIR_ENTRY_AMOUNT];
    disk.read(current_dir.block,(uint8_t*)&file_array);

    //Check if path is backtracing
    if (option == ".."){
        block_to_access = file_array[0].first_blk;
    }
    // Check if path is in current directory
    if (option == ""){
        block_to_access = current_dir.block;
    }
    if (option == "."){
        block_to_access = current_dir.block;
    }
    //if path is absolute
    if (org_path == "/"){
        return ROOT_BLOCK;
    }

    if (seperator_index == -1){
        return block_to_access;
    }

    else {
        //Reading in the active block and looping through the dir entries
        for (int i = is_root(block_to_access); i < DIR_ENTRY_AMOUNT; i++){
            if (strcmp(file_array[i].file_name,option.c_str()) == 0){
                block_to_access = file_array[i].first_blk;
                break;
            }
        }
    }


    //Cheking if enterd path is valid
    int path_found = 0;
    while (strcmp(path.c_str(),"") != 0){
        disk.read(block_to_access,(uint8_t*)&file_array);
        path_found = 0;
        seperator_index = path.find_first_of("/");

        //Check if path is only one block meaning we are done
        if (seperator_index == -1){
            option = path;
            path = "";
        }else{
            option = path.substr(0,seperator_index);
            path = path.substr(seperator_index+1);
        }

        if (option == ".."){
            block_to_access = file_array[0].first_blk;
            path_found = 1;
        }else{
        //Reading in the active block and looping through the dir entries
        for (int i = is_root(block_to_access); i < DIR_ENTRY_AMOUNT; i++){
            if (strcmp(file_array[i].file_name,option.c_str()) == 0){
                if((file_array[i].access_rights & READ) == 0){
                    std::cout << "Accesses not allowed";
                    return -1;
                }
                block_to_access = file_array[i].first_blk;
                path_found = 1;
                //Checking if the path contains files
                if (file_array[i].type == TYPE_FILE){
                    std::cout << "Error path contains files in path...\n";
                    return -1;
                }
                break;
            }
        }
        }
        //If the path is not found return -1
        if (path_found == 0){
            return -1;
        }
    }
    return block_to_access;
}

// Function to check if name is valid by checking lenght and if it is empty
int
FS::is_name_valid(std::string name){
    if (name.length() >= 56){
        std::cout << "Error name is too long\n";
        return -1;
    }
    else if (name.length() == 0)
    {
        std::cout << "Error name is empty\n";
        return -1;
    }
    return 0;
}

// Function to convert access rights from int to string acronym
std::string FS::access_int_to_acronym(int access_int) {
    std::string access_acronym;
    switch (access_int) {
        case 0:
            access_acronym = "---";
            break;
        case 1:
            access_acronym = "--x";
            break;
        case 2:
            access_acronym = "-w-";
            break;
        case 3:
            access_acronym = "-wx";
            break;
        case 4:
            access_acronym = "r--";
            break;
        case 5:
            access_acronym = "r-x";
            break;
        case 6:
            access_acronym = "rw-";
            break;
        case 7:
            access_acronym = "rwx";
            break;
        default:
            std::cout << "Error: invalid access rights\n";
            return "-1";
    }
    return access_acronym;
}

// Function to wipe the entire disk
int
FS::full_format()
{
    int empt_block[BLOCK_SIZE]{};
    for (int i = 0; i < NO_BLOCKS; i++){
        disk.write(i,(uint8_t*)&empt_block);
    }
    return 0;
}

//Program functions
FS::FS()
{
    std::cout << "FS::FS()... Creating file system\n";
    disk.read(FAT_BLOCK,(uint8_t*)fat);
    current_dir.block = ROOT_BLOCK;
}

FS::~FS()
{
    disk.write(FAT_BLOCK,(uint8_t*)fat);
    current_dir.block = ROOT_BLOCK;
}


// formats the disk, i.e., creates an empty file system
int
FS::format()
{
    for(int i = 0; i < NO_BLOCKS; i++){
        fat[i] = FAT_FREE;
    }
    fat[ROOT_BLOCK] = FAT_EOF;
    fat[FAT_BLOCK] = FAT_EOF;
    current_dir.block = ROOT_BLOCK;
    dir_entry array[DIR_ENTRY_AMOUNT]{};
    array[ROOT_BLOCK].access_rights = READ | WRITE | EXECUTE;
    disk.write(ROOT_BLOCK,(uint8_t*)&array);

    // full_format();

    return 0;
}

// create <filepath> creates a new file on the disk, the data content is
// written on the following rows (ended with an empty row)
int
FS::create(std::string filepath)
{
    std::string pre_path;
    std::string filename;
    //checking and separating path to file and path and also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(filepath,&filename,&pre_path);
    int active_block = get_subdiretory_from_path(pre_path);
    if (active_block == -1){
        std::cout << "Error path not found...\n";
        return -1;
    }

    //Check if name is valid
    if (is_name_valid(filename) == -1){
        return -1;
    }


    int save_free_index = -1;
    dir_entry file_array[DIR_ENTRY_AMOUNT];
    disk.read(active_block,(uint8_t*)&file_array);

    //Searching if there is space in directory, if not return -1
    for (int i = is_root(active_block); i < DIR_ENTRY_AMOUNT; i++){
        if (strcmp(file_array[i].file_name,filename.c_str()) == 0){
            std::cout << "Error file already exists...\n";
            return -1;
        }else if (strcmp(file_array[i].file_name,"") == 0){
            if (active_block != ROOT_BLOCK){
                if ((file_array[ROOT_BLOCK].access_rights & WRITE) == 0){ //Checking if we have write access rights
                    std::cout << "Error no write access rights...\n";
                    return -1;
                }
            }
            save_free_index = i;
            break;
        }

    }
    if (save_free_index == -1){
        std::cout << "Error no free space...\n";
        return -1;
    }

    //Reading in the user inputed file data
    std::string file_data;
    std::string row;
    int block_to_place_in;
    do {
        std::getline(std::cin, row);
        file_data += row + "\n";
        }while (!row.empty());
        file_data.pop_back(); //Remove last \n

    int amount_of_blocks = ((file_data.size()+1)/BLOCK_SIZE)+1; //Added null terminatro to string size

    //Create new dir entry
    dir_entry new_file;
    strcpy(new_file.file_name,filename.c_str());
    new_file.size = file_data.size();
    new_file.type = TYPE_FILE;
    new_file.access_rights = READ | WRITE | EXECUTE;


    //Finding a new datablock to place the file in
    int free_block = find_free_block();
    if (free_block != -1){
        new_file.first_blk = free_block;
    }else{
        std::cout << "Error no free blocks...\n";
        return -1;
    }
    //Write dir entry to disk
    int current_block = new_file.first_blk;
    int next_free_block;

    //Read in the active block
    file_array[save_free_index] = new_file;
    disk.write(active_block,(uint8_t*)file_array);

    //If file is larger than one block
    if (amount_of_blocks > 1){
        for (int i = 0; i < amount_of_blocks; i++){
            disk.write(current_block,(uint8_t*)file_data.substr(i*BLOCK_SIZE,BLOCK_SIZE).c_str());
            next_free_block = find_free_block();
            if (next_free_block == -1){
                std::cout << "Error ran out of memory...\n";
                return -1;
            }
            //Cheack if there is a free block and memory isn't full
            if (next_free_block != -1){
                fat[current_block] = next_free_block;
                current_block = next_free_block;
            }else{
                fat[current_block] = FAT_EOF;
                std::cout << "Error no free blocks...\n";
                return -1;
            }

            //If we are on the last block in the chain, set the last block to EOF
            if (i == amount_of_blocks-1){
                fat[current_block] = FAT_EOF;
            }
        }
    }else{
        disk.write(new_file.first_blk,(uint8_t*)file_data.c_str());
        fat[new_file.first_blk] = FAT_EOF;
    }
    return 0;
}


// cat <filepath> reads the content of a file and prints it on the screen
int
FS::cat(std::string filepath)
{
    std::string pre_path;
    std::string filename;
    //checking and separating path to file and path and also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(filepath,&filename,&pre_path);
    int active_block = get_subdiretory_from_path(pre_path);
    if (active_block == -1){
        std::cout << "Error path not found...\n";
        return -1;
    }

    //Read in the active block
    int file_found = 0;
    char file_data[BLOCK_SIZE];
    dir_entry file_array[DIR_ENTRY_AMOUNT]{};
    disk.read(active_block,(uint8_t*)&file_array);

    //Searching for file in subdirectory/root directory
    for (int i = is_root(active_block); i < DIR_ENTRY_AMOUNT; i++){
        if (strcmp(file_array[i].file_name,filename.c_str())==0){
            if ((file_array[i].access_rights & 1) == 0){ //Checking if we have read access rights
                std::cout << "Error no access rights...\n";
                return -1;
            }

                //check that the file is a file and not a directory
                if (file_array[i].type == TYPE_DIR){
                        std::cout << "Error destenation is not a file...\n";
                        return -1;
                }

                //Read the file differently if it is on the last block or not. If not on last block, add the next block to the read
                int block_to_read = file_array[i].first_blk;
                    if (fat[block_to_read] == FAT_EOF){
                        disk.read(block_to_read,(uint8_t*)&file_data);
                        std::cout << file_data;
                        return 0;
                    }
                    else{
                        do{
                            disk.read(block_to_read,(uint8_t*)&file_data);
                            std::cout << file_data;
                            block_to_read = fat[block_to_read];
                        }while (fat[block_to_read] != FAT_EOF);
                    }
                    return 0;

            }
    }
    //If file not found, raise error
    if (file_found == 0){
        std::cout << "Error file not found...\n";
        return -1;
    }

    return 0;
}


// ls lists the content in the currect directory (files and sub-directories)
int
FS::ls()
{
    //Reading in the active block
    std::string entry_type;
    std::string access_acronym;
    dir_entry file_array[DIR_ENTRY_AMOUNT]{};
    disk.read(current_dir.block,(uint8_t*)&file_array);

    //Printing the header
    std::cout << "name\ttype\taccessrights\tsize\n";
    int start_block = 1;

    //Looping through the root/sub directory that is our current standing block
    for (int i = is_root(current_dir.block); i < DIR_ENTRY_AMOUNT; i++){
        if (strcmp(file_array[i].file_name,"") != 0){
            if (file_array[i].access_rights & READ == 0){ //Checking if we have read access rights
                std::cout << "No read access rights\n";
                return -1;
            }

            //Checking if we have acsess to view the file, tror det är mycket effektivare o kolla
            access_acronym = access_int_to_acronym(file_array[i].access_rights);
            if (access_acronym == "-1"){
                return -1;
            } else {
                if (file_array[i].type == TYPE_FILE){
                    std::cout << file_array[i].file_name << "\tfile\t" << access_acronym << "\t\t" << file_array[i].size << "\n";

                }
                else if (file_array[i].type == TYPE_DIR){
                    std::cout << file_array[i].file_name << "\tdir\t" << access_acronym << "\t\t-\n";

                }
            }


        }

    }
    return 0;
}

// cp <sourcepath> <destpath> makes an exact copy of the file
// <sourcepath> to a new file <destpath>
int
FS::cp(std::string sourcepath, std::string destpath)
{
    std::string pre_path_source;
    std::string filename_source;
    //checking and separating path to file and path and also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(sourcepath,&filename_source,&pre_path_source);
    int active_block_source = get_subdiretory_from_path(pre_path_source);
    if (active_block_source == -1){
        std::cout << "Error path not found...\n";
        return -1;
    }

    std::string pre_path_dest;
    std::string filename_dest;
    //checking and separating path to file and path and also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(destpath,&filename_dest,&pre_path_dest);
    int active_block_dest = get_subdiretory_from_path(pre_path_dest);
    if (active_block_dest == -1){
        std::cout << "Path not found\n";
        return -1;
    }

    //Setting initial values & reading in the active block for the source file
    dir_entry file_array[DIR_ENTRY_AMOUNT]{};
    disk.read(active_block_source,(uint8_t*)&file_array);
    int source_block_to_read;
    int source_index = -1;
    int dest_index = -1;
    int file_dest_index = -1;
    int dest_dir_entry_index = -1;
    int file_type = TYPE_FILE;

    //Searching for souce file in directory
    for (int i = is_root(active_block_source); i < DIR_ENTRY_AMOUNT; i++){
        if (strcmp(file_array[i].file_name, filename_source.c_str()) == 0){
            if ((file_array[i].access_rights & READ) == 0){
                std::cout << "No read access rights\n";
                return -1;
            } else{

                source_block_to_read = file_array[i].first_blk;
                source_index = i;
            }
        }
        else if (strcmp(file_array[i].file_name, "") == 0){
            file_dest_index = i;
        }
    }

    // If source file is not found return -1
    if (source_index == -1){
        std::cout << "Sourcefile not found\n";
        return -1;
    }


    //Create new dir entry
    dir_entry new_file;
    new_file.size = file_array[source_index].size;
    new_file.type = TYPE_FILE;
    new_file.access_rights = file_array[source_index].access_rights;

    int free_block = find_free_block();
    if (free_block != -1){
        new_file.first_blk = free_block;
    }else{
        std::cout << "No free blocks\n";
        return -1;
    }
    int dest_curr_block = new_file.first_blk;
    int next_free_block;




    dir_entry file_array_dest[DIR_ENTRY_AMOUNT]{};
    disk.read(active_block_dest,(uint8_t*)&file_array_dest);

    // Searching for directory to place file in
    for (int i = is_root(active_block_dest); i < DIR_ENTRY_AMOUNT; i++){
        if (strcmp(file_array_dest[i].file_name,filename_dest.c_str()) == 0){
            if (file_array_dest[i].type == TYPE_FILE){
                std::cout << "File already exists\n";
                return -1;
            } else {
                file_type = TYPE_DIR;
                dest_dir_entry_index = file_array_dest[i].first_blk;
            }
        }
    }

    if (filename_dest == ".." || file_type == TYPE_DIR){
        strcpy(new_file.file_name,filename_source.c_str());
    

    } else {
        strcpy(new_file.file_name,filename_dest.c_str());
    }

    if (file_type != TYPE_FILE){
        // Reading header block of directory to place file in
        dir_entry dest_dir_entry_array[DIR_ENTRY_AMOUNT]{};
        disk.read(dest_dir_entry_index,(uint8_t*)&dest_dir_entry_array);

    // In directory to place file in, searching for free space
        for (int i = is_root(dest_dir_entry_index); i < DIR_ENTRY_AMOUNT; i++){
            if (strcmp(dest_dir_entry_array[i].file_name,"") == 0){
                dest_index = i;
                break;
            }
        }
        //Write dir entry to disk in new directory
        dest_dir_entry_array[dest_index] = new_file;
        disk.write(dest_dir_entry_index,(uint8_t*)dest_dir_entry_array);
    } else {
        //Write dir entry to disk in new directory
        file_array_dest[file_dest_index] = new_file;
        disk.write(active_block_dest,(uint8_t*)file_array_dest);
    }



    int i = 0;
    char file_data[BLOCK_SIZE];

    // Checking if file is only one block
    //Otherwise we need to loop through the blocks until we reach EOF

    if (fat[source_block_to_read] == FAT_EOF){
        disk.read(source_block_to_read,(uint8_t*)file_data);
        disk.write(dest_curr_block,(uint8_t*)file_data);
        fat[dest_curr_block] = FAT_EOF;
        return 0;
    } else {
        std::cout << "File is multiple blocks\n";
        while (fat[source_block_to_read] != FAT_EOF){
            disk.read(source_block_to_read,(uint8_t*)&file_data);
            char substring[BLOCK_SIZE + 1];  // +1 for the null terminator
            std::memcpy(substring, file_data + (i*BLOCK_SIZE), BLOCK_SIZE);
            substring[i*BLOCK_SIZE] = '\0';  // Null-terminate the substring
            disk.write(dest_curr_block,(uint8_t*)substring);

            next_free_block = find_free_block();
            if (next_free_block != -1){
                fat[dest_curr_block] = next_free_block;
                dest_curr_block = next_free_block;
            } else {
                fat[dest_curr_block] = FAT_EOF;
                std::cout << "No free blocks\n";
                return -1;
            }
            source_block_to_read = fat[source_block_to_read];
            i++;

        }
    }


    return 0;
}

// mv <sourcepath> <destpath> renames the file <sourcepath> to the name <destpath>,
// or moves the file <sourcepath> to the directory <destpath> (if dest is a directory)
int
FS::mv(std::string sourcepath, std::string destpath)
{

    std::string pre_path_source;
    std::string filename_source;
    //checking and separating path to file and path and also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(sourcepath,&filename_source,&pre_path_source);
    int active_block_source = get_subdiretory_from_path(pre_path_source);
    if (active_block_source == -1){
        std::cout << "Path not found\n";
        return -1;
    }

    std::string pre_path_dest;
    std::string destname;
    //checking and separating path to file and path and also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(destpath,&destname,&pre_path_dest);
    int active_block_dest = get_subdiretory_from_path(pre_path_dest);
    if (active_block_dest == -1){
        std::cout << "Path not found\n";
        return -1;
    }


    dir_entry file_array_source[DIR_ENTRY_AMOUNT];
    disk.read(active_block_source,(uint8_t*)file_array_source);

    // Searching for source file in directory
    int source_dir_entry_index = -1;
    for (int i = is_root(active_block_source); i < DIR_ENTRY_AMOUNT; i++){
        if (strcmp(file_array_source[i].file_name,filename_source.c_str()) == 0){
            if ( (file_array_source[i].access_rights & READ) == 0  && (file_array_source[i].access_rights & WRITE) == 0) {
                std::cout << "No read and write access rights\n";
                return -1;
            } else{
                source_dir_entry_index = i;
            }
        } else if (file_array_source[i].file_name,destname.c_str() == 0){
            return -1;
        }
    }



    dir_entry file_array_dest[DIR_ENTRY_AMOUNT];
    disk.read(active_block_dest,(uint8_t*)file_array_dest);


    int save_free_index = -1;
    int dir_block = -1;
    int dest_path_type = TYPE_FILE;

    // Searching for destination file in directory
    for (int i = is_root(active_block_dest); i < DIR_ENTRY_AMOUNT; i++){
        if (strcmp(file_array_dest[i].file_name,destname.c_str()) == 0){
            if (file_array_dest[i].type == TYPE_DIR) {
                if ((file_array_dest[active_block_dest].access_rights & WRITE) == 0){
                    std::cout << "No write access rights\n";
                    return -1;
                }
                dest_path_type = TYPE_DIR;
                dir_block = file_array_dest[i].first_blk;
            } else {
                std::cout << "File already exists!\n";
                return -1;
            }
        }
    }

    // IF destination is a file, we just rename the file
    if (dest_path_type == TYPE_FILE && destname != ".."){

        strcpy(file_array_source[source_dir_entry_index].file_name, destpath.c_str());
        disk.write(active_block_source,(uint8_t*)file_array_source);

    // If destination is previous directory, we move the file to the previous directory
    } else if (destname == ".."){
        dir_entry new_file_array[DIR_ENTRY_AMOUNT];
        disk.read(active_block_dest,(uint8_t*)new_file_array);
        for (int i = 0; i < DIR_ENTRY_AMOUNT; i++){
            if (strcmp(new_file_array[i].file_name,"") == 0){
                save_free_index = i;
                break;
            }
        }
        new_file_array[save_free_index] = file_array_source[source_dir_entry_index];
        disk.write(active_block_dest,(uint8_t*)new_file_array);

        dir_entry empty_entry;
        file_array_source[source_dir_entry_index] = empty_entry;
        disk.write(active_block_source,(uint8_t*)file_array_source);

    // If destination is a directory, we move the file to the directory
    } else {
        dir_entry new_file_array[DIR_ENTRY_AMOUNT];
        disk.read(dir_block,(uint8_t*)new_file_array);
        for (int i = 0; i < DIR_ENTRY_AMOUNT; i++){
            if (strcmp(new_file_array[i].file_name,"") == 0){
                save_free_index = i;
                break;
            }
        }
        new_file_array[save_free_index] = file_array_source[source_dir_entry_index];
        disk.write(dir_block,(uint8_t*)new_file_array);

        dir_entry empty_entry;
        file_array_source[source_dir_entry_index] = empty_entry;
        disk.write(active_block_source,(uint8_t*)file_array_source);


    }

    return 0;
}


// rm <filepath> removes / deletes the file <filepath>
int
FS::rm(std::string filepath)
{
    std::string pre_path;
    std::string filename;
    //checking and separating path to file and path and also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(filepath,&filename, &pre_path);
    int active_block = get_subdiretory_from_path(pre_path);
    if (active_block == -1){
        std::cout << "Path not found\n";
        return -1;
    }


    dir_entry file_array[DIR_ENTRY_AMOUNT];
    disk.read(active_block,(uint8_t*)file_array);

    int dir_entry_index = -1;

    // Searching for file in directory
    for (int i = is_root(active_block); i < DIR_ENTRY_AMOUNT; i++){
        if (strcmp(file_array[i].file_name, filename.c_str()) == 0){
            if ((file_array[i].access_rights & WRITE) == 0){
                std::cout << "No write access rights\n";
                return -1;
            }
            dir_entry_index = i;
            break;
        }
    }
    if (dir_entry_index == -1){
        std::cout << "File not found\n";
        return -1;
    }

    int block_to_read = file_array[dir_entry_index].first_blk;
    char zeros[BLOCK_SIZE]{};  // Initialize array with zeros

    // Checking if file is only one block
    if (fat[block_to_read] == FAT_EOF){
        if ((file_array[dir_entry_index].access_rights & WRITE) == 0){
            std::cout << "No write access rights\n";
            return -1;
        }
        disk.write(block_to_read,(uint8_t*)zeros); // Write zeros to the block
        file_array[dir_entry_index] = {}; // Set the dir entry to empty

        disk.write(active_block,(uint8_t*)file_array); // Update the directory
        fat[block_to_read] = FAT_FREE;
        return 0;
    }
    // If file is multiple blocks, we need to loop through the blocks until we reach EOF
    else {
        while (fat[block_to_read] != FAT_EOF){
            disk.write(block_to_read,(uint8_t*)&zeros);
            fat[block_to_read] = FAT_FREE;
            block_to_read = fat[block_to_read];
        }
    }



    return 0;
}

// append <filepath1> <filepath2> appends the contents of file <filepath1> to
// the end of file <filepath2>. The file <filepath1> is unchanged.
int
FS::append(std::string filepath1, std::string filepath2)
{
    std::string pre_path_1;
    std::string filename_1;
    //checking and separating path to file and path and also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(filepath1,&filename_1,&pre_path_1);
    int active_block_1 = get_subdiretory_from_path(pre_path_1);
    if (active_block_1 == -1){
        std::cout << "Error path not found...\n";
        return -1;
    }

    //Check if name is valid
    if (is_name_valid(filename_1) == -1){
        return -1;
    }

    std::string pre_path_2;
    std::string filename_2;
    //checking and separating path to file and path and also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(filepath2,&filename_2,&pre_path_2);
    int active_block_2 = get_subdiretory_from_path(pre_path_2);
    if (active_block_2 == -1){
        std::cout << "Error path not found...\n";
        return -1;
    }

    //Check if name is valid
    if (is_name_valid(filename_2) == -1){
        return -1;
    }

    // **Locate index of file 1**

    dir_entry file_array_1[DIR_ENTRY_AMOUNT];
    disk.read(active_block_1,(uint8_t*)file_array_1);
    int dir_entry_index_1 = -1;

    //Searching for file in directory
    for (int i = is_root(active_block_1); i < DIR_ENTRY_AMOUNT; i++){
        if (strcmp(file_array_1[i].file_name,filename_1.c_str()) == 0){
            if ((file_array_1[i].access_rights & READ) == 0){
                std::cout << "No read access rights\n";
                return -1;
            }
            if (file_array_1[i].type == TYPE_DIR){
                std::cout << "Error source is not a file...\n";
                return -1;
            } else {
                dir_entry_index_1 = i;
                break;
            }
        }
    }

    // **Locate index of file 2**
    dir_entry file_array_2[DIR_ENTRY_AMOUNT];
    disk.read(active_block_2,(uint8_t*)file_array_2);
    int dir_entry_index_2 = -1;

    //Searching for file in directory
    for (int i = is_root(active_block_2); i < DIR_ENTRY_AMOUNT; i++){
        if (strcmp(file_array_2[i].file_name,filename_2.c_str()) == 0){
            if ((file_array_2[i].access_rights & WRITE) == 0){
                std::cout << "No write access rights\n";
                return -1;
            }
            if (file_array_2[i].type == TYPE_DIR){
                std::cout << "Error source is not a file...\n";
                return -1;
            } else {
                dir_entry_index_2 = i;
                break;
            }
        }
    }

    // Extracting the first block of both files
    int block_to_read_1 = file_array_1[dir_entry_index_1].first_blk;
    int block_to_read_2 = file_array_2[dir_entry_index_2].first_blk;
    char file_data_1[BLOCK_SIZE];
    char file_data_2[BLOCK_SIZE];

    // Checking if file 1 is only one block
    if (fat[block_to_read_2] == FAT_EOF){
        disk.read(block_to_read_2,(uint8_t*)&file_data_2);
        std::string file_2_str(file_data_2);
        // This truly appends the two files, but the testfiles dont have the same format
        // if (file_2_str.find_first_of("\n") != std::string::npos){
        //     file_2_str.erase(file_2_str.find_first_of("\n"));
        // }

        if (fat[block_to_read_1] == FAT_EOF){
            disk.read(block_to_read_1,(uint8_t*)&file_data_1);
            std::string file_1_str(file_data_1);
            std::string appended_file = file_2_str + file_1_str;
            disk.write(block_to_read_2,(uint8_t*)appended_file.c_str());
            file_array_2[dir_entry_index_2].size = appended_file.size();
            disk.write(active_block_2,(uint8_t*)file_array_2);
            return 0;
        } else {
            do{
                disk.read(block_to_read_1,(uint8_t*)&file_data_1);
                std::string file_1_str(file_data_1);
                if (file_1_str.find_first_of("\n") != std::string::npos){
                    file_1_str.erase(file_1_str.find_first_of("\n"));
                }
                std::string appended_file = file_2_str + file_1_str;
                disk.write(block_to_read_2,(uint8_t*)appended_file.c_str());
                file_array_2[dir_entry_index_2].size = appended_file.size();
                disk.write(active_block_2,(uint8_t*)file_array_2);
                block_to_read_1 = fat[block_to_read_1];
            }while (fat[block_to_read_1] != FAT_EOF);
            return 0;
        }
    // If file 1 is multiple blocks, we need to loop through the blocks until we reach EOF
    } else {
        do{
            block_to_read_2 = fat[block_to_read_2];
        }while (fat[block_to_read_2] != FAT_EOF);

        if (fat[block_to_read_1] == FAT_EOF){
            disk.read(block_to_read_1,(uint8_t*)&file_data_1);
            disk.read(block_to_read_2,(uint8_t*)&file_data_2);
            std::string file_1_str(file_data_1); // Converts both char arrays to strings
            std::string file_2_str(file_data_2);
            // This truly appends the two files, but the testfiles dont have the same format
            // if (file_2_str.find_first_of("\n") != std::string::npos){
            //     file_2_str.erase(file_2_str.find_first_of("\n"));
            // }
            std::string appended_file = file_2_str + file_1_str; // Appends the two strings
            disk.write(block_to_read_2,(uint8_t*)appended_file.c_str()); // Writes the appended string to the last block of file 2
            file_array_2[dir_entry_index_2].size = appended_file.size();
            disk.write(active_block_2,(uint8_t*)file_array_2);
            return 0;
        } else {
            do{
                disk.read(block_to_read_1,(uint8_t*)&file_data_1);
                disk.read(block_to_read_2,(uint8_t*)&file_data_2);
                std::string file_1_str(file_data_1);
                std::string file_2_str(file_data_2);
                // This truly appends the two files, but the testfiles dont have the same format
                // if (file_2_str.find_first_of("\n") != std::string::npos){
                //     file_2_str.erase(file_2_str.find_first_of("\n"));
                // }
                std::string appended_file = file_2_str + file_1_str;
                disk.write(block_to_read_2,(uint8_t*)appended_file.c_str());
                file_array_2[dir_entry_index_2].size = appended_file.size();
                disk.write(active_block_2,(uint8_t*)file_array_2);
                block_to_read_1 = fat[block_to_read_1];
            }while (fat[block_to_read_1] != FAT_EOF);
            return 0;
        }
    }
    return 0;
}

// mkdir <dirpath> creates a new sub-directory with the name <dirpath>
// in the current directory
int
FS::mkdir(std::string dirpath)
{
    std::string pre_path;
    std::string dirname;
    //checking and separating path to file and path and also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(dirpath,&dirname,&pre_path);
    int active_block = get_subdiretory_from_path(pre_path);
    if (active_block == -1){
        std::cout << "Error path not found...\n";
        return -1;
    }

    //Check if name is valid
    if (is_name_valid(dirname) == -1){
        return -1;
    }

    dir_entry file_array[DIR_ENTRY_AMOUNT]{};
    disk.read(active_block,(uint8_t*)&file_array);

    if ((file_array[ROOT_BLOCK].access_rights & READ) == 0 && (file_array[ROOT_BLOCK].access_rights & WRITE) == 0){
                std::cout << "No read and write access rights\n";
                return -1;
            }

    int save_free_index = -1;
    for (int i = is_root(active_block); i < DIR_ENTRY_AMOUNT; i++){
        if (strcmp(file_array[i].file_name,dirname.c_str()) == 0){
            std::cout << "Error name already exists...\n";
            return -1;
        }else if (strcmp(file_array[i].file_name,"") == 0){

            save_free_index = i;
            break;
        }

    }
    if (save_free_index == -1){
        std::cout << "Error no free space...\n";
        return -1;
    }

    int directory_block = find_free_block();
    //Create new dir entry
    dir_entry new_dir;
    strcpy(new_dir.file_name,dirname.c_str());
    new_dir.size = 0;
    new_dir.type = TYPE_DIR;
    new_dir.access_rights = READ | WRITE | EXECUTE;
    new_dir.first_blk = directory_block;

    //Write dir entry to disk
    file_array[save_free_index] = new_dir;
    disk.write(active_block,(uint8_t*)file_array);
    //Write to fat
    fat[directory_block] = FAT_EOF;
    //Write array to disk
    dir_entry parent_block;
    parent_block.first_blk = active_block;
    parent_block.access_rights = READ | WRITE | EXECUTE;
    parent_block.size = 0;
    parent_block.type = TYPE_DIR;
    strcpy(parent_block.file_name,dirname.c_str());

    dir_entry array[DIR_ENTRY_AMOUNT]{};
    array[0] = parent_block;
    disk.write(directory_block,(uint8_t*)array);
    return 0;
}

// cd <dirpath> changes the current (working) directory to the directory named <dirpath>
int
FS::cd(std::string dirpath)
{
    std::string pre_path;
    std::string dirname;
    //checking and separating path to file and path and also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(dirpath,&dirname,&pre_path);
    int active_block = get_subdiretory_from_path(pre_path);
    if (active_block == -1){
        std::cout << "Error path not found...\n";
        return -1;
    }
    int save_entry_index;
    int file_found = 0;
    dir_entry file_array[DIR_ENTRY_AMOUNT]{};


    disk.read(active_block,(uint8_t*)&file_array);

    if ((strcmp(dirpath.c_str(),"..") == 0)||(strcmp(dirname.c_str(),"") == 0)){
            current_dir.block = active_block;
        return 0;
    }else{
        //Looping from first entry excepth parent
        for (int i = is_root(active_block); i < DIR_ENTRY_AMOUNT; i++){
            if (strcmp(file_array[i].file_name,dirname.c_str()) == 0){
                if ((file_array[active_block].access_rights & READ) == 0){
                    std::cout << "No read access rights\n";
                    return -1;
                }
                save_entry_index = i;
                file_found = 1;
                if (file_array[save_entry_index].type == TYPE_FILE){
                    std::cout << "Error destination is not a directory, can't change directory...\n";
                    return -1;
                }
                else{
                    current_dir.block = file_array[save_entry_index].first_blk;
                    return 0;
                }
            }
        }
        std::cout << "error directory not found...\n";
    return -1;
    }
}

// pwd prints the full path, i.e., from the root directory, to the current
// directory, including the currect directory name
int
FS::pwd()
{
    std::string full_path;
    dir_entry file_array[DIR_ENTRY_AMOUNT]{};
    int copy_of_current_block = current_dir.block;

    if (copy_of_current_block == ROOT_BLOCK){
        full_path = "/";
    }else{
        while (copy_of_current_block != ROOT_BLOCK){
            disk.read(copy_of_current_block,(uint8_t*)&file_array);
            full_path = file_array[0].file_name+full_path;
            full_path = "/"+full_path;
            copy_of_current_block = file_array[0].first_blk;

        }
    }
    std::cout << full_path << "\n";
    return 0;
}

// chmod <accessrights> <filepath> changes the access rights for the
// file <filepath> to <accessrights>.
int
FS::chmod(std::string accessrights, std::string filepath)
{
    std::string pre_path;
    std::string filename;
    //checking and separating path to file and path and also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(filepath,&filename,&pre_path);
    int active_block = get_subdiretory_from_path(pre_path);
    if (active_block == -1){
        std::cout << "Error path not found...\n";
        return -1;
    }

    dir_entry file_array[DIR_ENTRY_AMOUNT]{};
    disk.read(active_block,(uint8_t*)&file_array);

    // Searching for file in directory
    int save_free_index = -1;
    for (int i = is_root(active_block); i < DIR_ENTRY_AMOUNT; i++){
        if (strcmp(file_array[i].file_name,filename.c_str()) == 0){
            std::cout << "File Found...\n";
            save_free_index = i;
            break;
            return -1;
        }
    }
    if (save_free_index == -1){
        std::cout << "Error file not found...\n";
        return -1;
    }

    // Checking if accessrights is valid
    if (accessrights.size() != 1){
        std::cout << "Error accessrights not valid...\n";
        return -1;
    } else if (std::stoi(accessrights) < 0 || std::stoi(accessrights) > 7) {
        std::cout << "Error accessrights not valid...\n";
        return -1;
    }

    // Updating accessrights
    file_array[save_free_index].access_rights = (uint8_t)std::stoi(accessrights);
    disk.write(active_block,(uint8_t*)file_array);

    return 0;
}