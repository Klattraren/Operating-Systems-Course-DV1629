#include <iostream>
#include "fs.h"
#include <cstring>

// Help functions
// finds a free block in the FAT system
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

void
FS::separate_name_dir(std::string filepath,std::string *filename,std::string *pre_path){
    int seperator_index = filepath.find_last_of("/");
    *filename = filepath.substr(seperator_index+1);
    *pre_path = filepath.substr(0,seperator_index);
    // std::cout << "filename is: " << filename <<"\n";
    // std::cout << "path is: " << pre_path << "\n";
    
}

int
FS::get_block_from_path(std::string path){
    
    int block_to_acsess = current_dir.block;
    int seperator_index = path.find_first_of("/");
    std::string option = path.substr(0,seperator_index);
    path = path.substr(seperator_index+1);
    
    //Reading in the block we stand in to change path
    dir_entry file_array[DIR_ENTRY_AMOUNT];
    disk.read(current_dir.block,(uint8_t*)&file_array);

    //Check if path is absolute or relative
    if (option == ".."){
        block_to_acsess = file_array[0].first_blk;
    }
    if (option == ""){
        block_to_acsess = ROOT_BLOCK;
    }
    if (option == "."){
        block_to_acsess = current_dir.block;
    }

    if (seperator_index == -1){
        return block_to_acsess;
    }

    //Cheking if enterd path is valid
    int path_found = 0;
    while (strcmp(path.c_str(),"") != 0){
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

        //Reading in the active block and looping through the dir entries
        disk.read(block_to_acsess,(uint8_t*)&file_array);
        for (int i = 1; i < DIR_ENTRY_AMOUNT; i++){
            if (strcmp(file_array[i].file_name,option.c_str()) == 0){
                block_to_acsess = file_array[i].first_blk;
                path_found = 1;
                break;
            }
        }

        //If the path is not found return -1
        if (path_found == 0){
            std::cout << "Path not found\n";
            return -1;
        }
    }
    return block_to_acsess;
}

int
FS::find_block_from_name(std::string filename)
{
    dir_entry file_array[DIR_ENTRY_AMOUNT];
    disk.read(current_dir.block,(uint8_t*)&file_array);
    for (int i = 1; i < DIR_ENTRY_AMOUNT; i++){
        // std::cout << "File name: " << file_array[i].file_name << "\n";
        if (strcmp(file_array[i].file_name,filename.c_str()) == 0){
            return i;
        }
    }
    return -1;
}


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
    std::cout << "FS::format()\n";
    for(int i = 0; i < NO_BLOCKS; i++){
        fat[i] = FAT_FREE;
    }
    fat[ROOT_BLOCK] = FAT_EOF;
    fat[FAT_BLOCK] = FAT_EOF;
    current_dir.block = ROOT_BLOCK;
    int array[BLOCK_SIZE];
    array[0] = 0;
    disk.write(ROOT_BLOCK,(uint8_t*)array);
    return 0;
}

// create <filepath> creates a new file on the disk, the data content is
// written on the following rows (ended with an empty row)
int
FS::create(std::string filepath)
{
    std::string pre_path;
    std::string filename;
    //checking and separating path to file and path adn also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(filepath,&filename,&pre_path);
    int active_block = get_block_from_path(pre_path);
    if (active_block == -1){
        std::cout << "Path not found\n";
        return -1;
    }

    int save_free_index = 0;
    dir_entry file_array[DIR_ENTRY_AMOUNT];
    disk.read(active_block,(uint8_t*)&file_array);

    for (int i = 0; i < DIR_ENTRY_AMOUNT; i++){
        if (strcmp(file_array[i].file_name,filepath.c_str()) == 0){
            std::cout << "File already exists\n";
            return -1;
        }else if (strcmp(file_array[i].file_name,"") == 0){
            save_free_index = i;
            break;
        }
        
    }

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
    std::cout << "FILE LENGTH: " << filename.length()<< "\n";
    if (filename.length() >= 56){
        std::cout << "Filename is too long\n";
        return -1;
    }
    strncpy(new_file.file_name,filename.c_str(),56);
    new_file.size = file_data.size();
    new_file.type = TYPE_FILE;

    int free_block = find_free_block();
    if (free_block != -1){
        new_file.first_blk = free_block;
    }else{
        std::cout << "No free blocks\n";
        return -1;
    }
    //Write dir entry to disk
    int current_block = new_file.first_blk;
    int next_free_block;

    file_array[save_free_index] = new_file;
    disk.write(active_block,(uint8_t*)file_array);

    if (amount_of_blocks > 1){ //If file is larger than one block
        for (int i = 0; i < amount_of_blocks; i++){ 
            disk.write(current_block,(uint8_t*)file_data.substr(i*BLOCK_SIZE,BLOCK_SIZE).c_str());
            next_free_block = find_free_block();
            //Cheack if there is a free block and memory isn't full
            if (next_free_block != -1){
                fat[current_block] = next_free_block;
                current_block = next_free_block;
            }else{
                fat[current_block] = FAT_EOF;
                std::cout << "No free blocks\n";
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
    //checking and separating path to file and path adn also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(filepath,&filename,&pre_path);
    int active_block = get_block_from_path(pre_path);
    if (active_block == -1){
        std::cout << "Path not found\n";
        return -1;
    }

    int start_block = 1;
    if (current_dir.block == ROOT_BLOCK){
        start_block = 0;}

    int file_found = 0;
    char file_data[BLOCK_SIZE];
    dir_entry file_array[DIR_ENTRY_AMOUNT];
    disk.read(active_block,(uint8_t*)&file_array);
    for (int i = start_block; i < DIR_ENTRY_AMOUNT; i++){
        if (strcmp(file_array[i].file_name,filename.c_str())==0){
            // std::cout << "File found\n";
           int block_to_read = file_array[i].first_blk;
            // std::cout << "FAT: " << fat[block_to_read] << "\n";
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
    if (file_found == 0){
        std::cout << "File not found\n";
        return -1;
    }
    
    return 0;
}


// ls lists the content in the currect directory (files and sub-directories)
int
FS::ls()
{
    // std::cout << "FS::ls()\n";
    std::string entry_type;
    dir_entry file_array[DIR_ENTRY_AMOUNT];
    disk.read(current_dir.block,(uint8_t*)&file_array);
    std::cout << "name\ttype\tsize\n"; 
    int start_block = 1;
    if (current_dir.block == ROOT_BLOCK){
        start_block = 0;}
        
    for (int i = start_block; i < DIR_ENTRY_AMOUNT; i++){
        if (strcmp(file_array[i].file_name,"") != 0){
            if (file_array[i].type == TYPE_FILE){
            std::cout << file_array[i].file_name << "\tfile\t" << file_array[i].size << "\n";

            }
            else if (file_array[i].type == TYPE_DIR){
                std::cout << file_array[i].file_name << "\tdir\t-\n";

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
    //checking and separating path to file and path adn also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(sourcepath,&filename_source,&pre_path_source);
    int active_block_source = get_block_from_path(pre_path_source);
    if (active_block_source == -1){
        std::cout << "Path not found\n";
        return -1;
    }
    std::cout << "pre_path_source: " << pre_path_source << "\n";
    std::cout << "Source block: " << active_block_source << "\n";

    std::string pre_path_dest;
    std::string filename_dest;
    //checking and separating path to file and path adn also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(destpath,&filename_dest,&pre_path_dest);
    int active_block_dest = get_block_from_path(pre_path_dest);
    if (active_block_dest == -1){
        std::cout << "Path not found\n";
        return -1;
    }
    std::cout << "pre_path_dest: " << pre_path_dest << "\n";
    std::cout << "Dest block: " << active_block_dest << "\n";

    int exist = 0;
    dir_entry file_array[DIR_ENTRY_AMOUNT];
    disk.read(active_block_source,(uint8_t*)&file_array);
    int source_block_to_read;
    int source_index;

    for (int i = 1; i < DIR_ENTRY_AMOUNT; i++){

        if (strcmp(file_array[i].file_name,filename_dest.c_str()) == 0){
            std::cout << "Filename" << destpath.c_str() << "already exist, please choose another name!";
            exist = 1;
            return -1;
        } else if (strcmp(file_array[i].file_name, filename_source.c_str()) == 0){
            source_block_to_read = file_array[i].first_blk;
            source_index = i;
            break;
        }
    }



    
    std::cout << "Source block to read: " << source_block_to_read << "\n";

    char file_data[BLOCK_SIZE];

    

    //Create new dir entry
    dir_entry new_file;
    strncpy(new_file.file_name,filename_dest.c_str(),56);
    new_file.size = file_array[source_index].size;
    std::cout << "Size: " << new_file.size << "\n";
    new_file.type = TYPE_FILE;

    int free_block = find_free_block();
    if (free_block != -1){
        new_file.first_blk = free_block;
    }else{
        std::cout << "No free blocks\n";
        return -1;
    }
    int dest_curr_block = new_file.first_blk;
    int next_free_block;

    //Write dir entry to disk
    file_array[dest_curr_block] = new_file;
    disk.write(active_block_dest,(uint8_t*)file_array);

    int source_next_block = fat[source_block_to_read];
    int i = 0;

    std::cout << "fat[source_block_index]: " << fat[source_block_to_read] << "\n";

    if (fat[source_block_to_read] == FAT_EOF){
        std::cout << "File is only one block\n";
        disk.read(source_block_to_read,(uint8_t*)&file_data);
        std::cout << "File data: " << file_data << "\n";
        disk.write(dest_curr_block,(uint8_t*)file_data);
        fat[dest_curr_block] = FAT_EOF;
        return 0;
    } else {
        std::cout << "File is multiple blocks\n";
        while (fat[source_block_to_read] != FAT_EOF){
            std::cout << "Copying block: " << source_block_to_read << "\n";
            disk.read(source_block_to_read,(uint8_t*)&file_data);
            std::cout << "File data: " << file_data << "\n";
            char substring[BLOCK_SIZE + 1];  // +1 for the null terminator
            std::memcpy(substring, file_data + (i*BLOCK_SIZE), BLOCK_SIZE);
            substring[i*BLOCK_SIZE] = '\0';  // Null-terminate the substring
            std::cout << "SUBSTRING: " << substring << "\n";
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




    std::cout << "FS::cp(" << sourcepath << "," << destpath << ")\n";
    return 0;
}

// mv <sourcepath> <destpath> renames the file <sourcepath> to the name <destpath>,
// or moves the file <sourcepath> to the directory <destpath> (if dest is a directory)
int
FS::mv(std::string sourcepath, std::string destname)
{

    std::string pre_path_source;
    std::string filename_source;
    //checking and separating path to file and path adn also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(sourcepath,&filename_source,&pre_path_source);
    int active_block_source = get_block_from_path(pre_path_source);
    if (active_block_source == -1){
        std::cout << "Path not found\n";
        return -1;
    }

    std::cout << "active_block_source: " << active_block_source << "\n";

    dir_entry file_array[DIR_ENTRY_AMOUNT];
    disk.read(active_block_source,(uint8_t*)file_array);
    
    int dir_entry_index = -1;

    std::cout << "file array: " << file_array[0].file_name << "\n";

    for (int i = 1; i < DIR_ENTRY_AMOUNT; i++){
        if (strcmp(file_array[i].file_name,filename_source.c_str()) == 0){
            std::cout << "Source file found\n";
            dir_entry_index = i;
        } else if (file_array[i].file_name,destname.c_str() == 0){
            std::cout << "Destination path already exists!\n";
            return -1;
        } 
    }
    int source_block_to_read = file_array[dir_entry_index].first_blk;
    std::cout << "Dir index: " << dir_entry_index << "\n";
    std::cout << "Source block to read: " << source_block_to_read << "\n";

    if (file_array[source_block_to_read].type == TYPE_DIR){
        std::cout << "Source is a directory!\n";
        return -1;
    } else if  (file_array[source_block_to_read].type == TYPE_FILE){
        std::cout << "Source is a file!\n";
        std::cout << "Source index: " << dir_entry_index << "\n";
        strncpy(file_array[dir_entry_index].file_name, destname.c_str(),56);
        std::cout << "New file name: " << file_array[dir_entry_index].file_name << "\n";
        disk.write(active_block_source,(uint8_t*)file_array);

    } else {
        std::cout << "Invalid file type!\n";
        return -1;
    }

    std::cout << "FS::mv(" << sourcepath << "," << destname << ")\n";
    return 0;
}


// rm <filepath> removes / deletes the file <filepath>
int
FS::rm(std::string filepath)
{
    std::string pre_path;
    std::string filename;
    //checking and separating path to file and path adn also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(filepath,&filename, &pre_path);
    int active_block = get_block_from_path(pre_path);
    if (active_block == -1){
        std::cout << "Path not found\n";
        return -1;
    }

    std::cout << "active_block_source: " << active_block << "\n";


    dir_entry file_array[DIR_ENTRY_AMOUNT];
    disk.read(active_block,(uint8_t*)file_array);
    
    int dir_entry_index = -1;

    for (int i = 1; i < DIR_ENTRY_AMOUNT; i++){
        std::cout << "File name: " << file_array[i].file_name << "\n";
        if (strcmp(file_array[i].file_name, filename.c_str()) == 0){
            std::cout << "File found\n";
            dir_entry_index = i;
            break;
        }  
    }

    int block_to_read = file_array[dir_entry_index].first_blk;
    char zeros[BLOCK_SIZE] = {0};  // Initialize array with zeros
    std::cout << "Block to read: " << block_to_read << "\n";

    if (fat[block_to_read] == FAT_EOF){
        std::cout << "File is only one block\n";
        disk.write(block_to_read,(uint8_t*)&zeros);

        file_array[active_block] = dir_entry();

        disk.write(active_block,(uint8_t*)file_array);
        fat[block_to_read] = FAT_FREE;
        std::cout << "File deleted\n";
        return 0;
    } 
    // else {
    //     std::cout << "File is multiple blocks\n";
    //     while (fat[block_to_read] != FAT_EOF){
    //         std::cout << "Deleting block: " << block_to_read << "\n";
    //         disk.write(block_to_read,(uint8_t*)&zeros);
    //         fat[block_to_read] = FAT_FREE;
    //         block_to_read = fat[block_to_read];
    //     }
    // }



    std::cout << "FS::rm(" << filepath << ")\n";
    return 0;
}

// append <filepath1> <filepath2> appends the contents of file <filepath1> to
// the end of file <filepath2>. The file <filepath1> is unchanged.
int
FS::append(std::string filepath1, std::string filepath2)
{
    std::cout << "FS::append(" << filepath1 << "," << filepath2 << ")\n";
    return 0;
}

// mkdir <dirpath> creates a new sub-directory with the name <dirpath>
// in the current directory
int
FS::mkdir(std::string dirpath)
{
    int save_free_index = 1;
    dir_entry file_array[DIR_ENTRY_AMOUNT];
    int block = current_dir.block;
    disk.read(current_dir.block,(uint8_t*)&file_array);
    for (int i = 1; i < DIR_ENTRY_AMOUNT; i++){
        // std::cout << "\nFile name: " << file_array[i].file_name << "\n";
        if (strcmp(file_array[i].file_name,dirpath.c_str()) == 0){
            std::cout << "Directory already exists\n";
            return -1;
        }else if (strcmp(file_array[i].file_name,"") == 0){
            save_free_index = i;
            // std::cout << "Free index: " << save_free_index << "\n";
            break;
        }
        
    }

    int directory_block = find_free_block();
    //Create new dir entry
    dir_entry new_dir;
    strncpy(new_dir.file_name,dirpath.c_str(),56);
    new_dir.size = 0;
    new_dir.type = TYPE_DIR;
    new_dir.first_blk = directory_block;

    //Write dir entry to disk
    file_array[save_free_index] = new_dir;
    disk.write(current_dir.block,(uint8_t*)file_array);
    //Write to fat
    fat[directory_block] = FAT_EOF;
    //Write array to disk
    dir_entry parent_block;
    parent_block.first_blk = current_dir.block;
    parent_block.size = 0;
    parent_block.type = TYPE_DIR;
    strncpy(parent_block.file_name,dirpath.c_str(),56);

    dir_entry array[BLOCK_SIZE];
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
    //checking and separating path to file and path adn also abolute/relative
    //Returning the block to place the file in or -1 if path is not valid
    separate_name_dir(dirpath,&dirname,&pre_path);
    int active_block = get_block_from_path(pre_path);
    if (active_block == -1){
        std::cout << "Path not found\n";
        return -1;
    }

    int save_entry_index;
    int file_found = 0;
    dir_entry file_array[DIR_ENTRY_AMOUNT];

    disk.read(active_block,(uint8_t*)&file_array);
    if (strcmp(dirname.c_str(),"..") == 0){
            current_dir.block = active_block;
        return 0;
    }else{
        //Looping from first entry excepth parent
        for (int i = 0; i < DIR_ENTRY_AMOUNT; i++){
            if (strcmp(file_array[i].file_name,dirname.c_str()) == 0){
                save_entry_index = i;
                file_found = 1;
                break;
            }
        }
        if (file_found == 0){
        std::cout << "directory not found\n";
        return -1;}
        
    // }
    //Take out the block number of the directory
    current_dir.block = file_array[save_entry_index].first_blk;
    // current_dir.block = active_block;

    return 0;
}
}

// pwd prints the full path, i.e., from the root directory, to the current
// directory, including the currect directory name
int
FS::pwd()
{
    std::string full_path;
    dir_entry file_array[DIR_ENTRY_AMOUNT];
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
    std::cout << "FS::chmod(" << accessrights << "," << filepath << ")\n";
    return 0;
}