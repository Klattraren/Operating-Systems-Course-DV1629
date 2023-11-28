#include <iostream>
#include "fs.h"
#include <cmath>

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

FS::FS()
{
    std::cout << "FS::FS()... Creating file system\n";
}

FS::~FS()
{

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
    std::cout << "FS::create(" << filepath << ")\n";
    std::string file_data;
    std::string row;
    int block_to_place_in;
    do {
        std::getline(std::cin, row);
        file_data += row + "\n";
        }while (!row.empty());

    std::cout << "File size: "<< file_data.size() << "\n";
    int amount_of_blocks = ((file_data.size()+1)/BLOCK_SIZE)+1; //Added null terminatro to string size
    std::cout << "Amount of blocks that will be used: " << amount_of_blocks << "\n";

    
    dir_entry new_file;
    new_file.size = file_data.size();
    new_file.first_blk = find_free_block();
    new_file.type = TYPE_FILE;
    disk.write(ROOT_BLOCK,(uint8_t*)&new_file);

    for (int i = 0;i<amount_of_blocks;i++){
        block_to_place_in = find_free_block();
        std::cout << "Block TO place in: " << block_to_place_in << "\n";
        if (block_to_place_in != -1){
            std::cout << "Placing BLock";
        }
    }
    
    return 0;
}

// cat <filepath> reads the content of a file and prints it on the screen
int
FS::cat(std::string filepath)
{
    std::cout << "FS::cat(" << filepath << ")\n";
    return 0;
}

// ls lists the content in the currect directory (files and sub-directories)
int
FS::ls()
{
    std::cout << "FS::ls()\n";
    return 0;
}

// cp <sourcepath> <destpath> makes an exact copy of the file
// <sourcepath> to a new file <destpath>
int
FS::cp(std::string sourcepath, std::string destpath)
{
    std::cout << "FS::cp(" << sourcepath << "," << destpath << ")\n";
    return 0;
}

// mv <sourcepath> <destpath> renames the file <sourcepath> to the name <destpath>,
// or moves the file <sourcepath> to the directory <destpath> (if dest is a directory)
int
FS::mv(std::string sourcepath, std::string destpath)
{
    std::cout << "FS::mv(" << sourcepath << "," << destpath << ")\n";
    return 0;
}

// rm <filepath> removes / deletes the file <filepath>
int
FS::rm(std::string filepath)
{
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
    std::cout << "FS::mkdir(" << dirpath << ")\n";
    return 0;
}

// cd <dirpath> changes the current (working) directory to the directory named <dirpath>
int
FS::cd(std::string dirpath)
{
    std::cout << "FS::cd(" << dirpath << ")\n";
    return 0;
}

// pwd prints the full path, i.e., from the root directory, to the current
// directory, including the currect directory name
int
FS::pwd()
{
    std::cout << "FS::pwd()\n";
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
