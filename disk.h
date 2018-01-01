#ifndef _DISK_H
#define _DISK_H

#include <string>
#include <fstream>

const int BLOCK_SIZE = 4096;
const int SUPER_BLOCK_NUM = 2;
const int INODE_BLOCK_NUM = 32;
const int DATA_BLOCK_NUM = 4096;
const int BLOCK_NUM =  SUPER_BLOCK_NUM + INODE_BLOCK_NUM + DATA_BLOCK_NUM;

const int FILE_MAX_BLOCKS = 1;
const int INODE_PLACEHOLDER_SIZE = 4;
const int MAX_FILENAME_SIZE = 252;
const int MAX_DIR_ENTRIES_IN_BLOCK = 16;

const char EMPTY_BYTE = 0x00;
const char FILLED_BYTE = 0x01;

struct super_block {
    bool inode_bitmap[BLOCK_SIZE];
    bool block_bitmap[BLOCK_SIZE];
};

struct inode {
    int i_id;
    int i_mode; // file or directory, 0 for file, 1 for dir
    int i_file_size;

    int i_blocks[FILE_MAX_BLOCKS];
    int i_placeholder[INODE_PLACEHOLDER_SIZE];
};

struct file_block {
    char data[BLOCK_SIZE];
};

struct dir_entry {
    char name[MAX_FILENAME_SIZE];
    int inode_id;
};

struct dir_block {
    dir_entry dirs[MAX_DIR_ENTRIES_IN_BLOCK];
};

struct disk_file {
    super_block *superBlockPointer;
    inode *inodePointer;
    file_block *fileBlockPointer;
    dir_block *dirBlockPointer;
    std::string inodeName[4096];
    bool blockType[4096];
};

// void writeSuperBlock(ofstream &fout, const super_block &block);
// void writeInode(ofstream &fout, const inode &block);
// void writeFileBlock(ofstream &fout, const file_block &block);
// void writeDirBlock(ofstream &fout, const dir_block &block);
// int getInt(ifstream &fin);
// super_block readSuperBlock(ifstream &fin);
// inode readInode(ifstream &fin);
// file_block readFileBlock(ifstream &fin);
// dir_block readDirBlock(ifstream &fin);
bool checkDisk(const std::string &fileName);
bool format(const std::string &fileName);
disk_file readDisk(std::ifstream &fin);
void writeDisk(std::ofstream &fout, const disk_file &disk);

#endif