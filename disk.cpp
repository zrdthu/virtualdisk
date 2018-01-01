#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include "disk.h"
using namespace std;

void writeInt(ofstream &fout, int num) {
    char buff[4];
    memcpy(buff, &num, sizeof(buff));
    fout.write(buff, 4);
}

void writeSuperBlock(ofstream &fout, const super_block &block) {
    for (int i = 0; i < BLOCK_SIZE; i ++)
        fout.put(block.inode_bitmap[i] ? FILLED_BYTE : EMPTY_BYTE);
    for (int i = 0; i < BLOCK_SIZE; i ++)
        fout.put(block.block_bitmap[i] ? FILLED_BYTE : EMPTY_BYTE);
}

void writeInode(ofstream &fout, const inode &block) {
    writeInt(fout, block.i_id);
    writeInt(fout, block.i_mode);
    writeInt(fout, block.i_file_size);
    
    for (int i = 0; i < FILE_MAX_BLOCKS; i ++)
        writeInt(fout, block.i_blocks[i]);
    for (int i = 0; i < INODE_PLACEHOLDER_SIZE; i ++)
        writeInt(fout, block.i_placeholder[i]);
}

void writeFileBlock(ofstream &fout, const file_block &block) {
    for (int i = 0; i < BLOCK_SIZE; i ++)
        fout.put(block.data[i]);
}

void writeDirBlock(ofstream &fout, const dir_block &block) {
    for (int i = 0 ; i < MAX_DIR_ENTRIES_IN_BLOCK; i ++) {
        for (int j = 0; j < MAX_FILENAME_SIZE; j ++)
            fout.put(block.dirs[i].name[j]);
        writeInt(fout, block.dirs[i].inode_id);
    }
}

void writeDisk(ofstream &fout, const disk_file &disk) {
    writeSuperBlock(fout, *disk.superBlockPointer);
    for (int i = 0; i < 4096; i ++)
        writeInode(fout, disk.inodePointer[i]);
    for (int i = 0; i < 4096; i ++)
        if (disk.blockType[i])
            writeDirBlock(fout, disk.dirBlockPointer[i]);
        else
            writeFileBlock(fout, disk.fileBlockPointer[i]);
}

int getInt(ifstream &fin) {
    char buff[4];
    fin.read(buff, sizeof(buff));
    int ans = 0;
    memcpy(&ans, buff, sizeof(ans));
    return ans;
}

super_block readSuperBlock(ifstream &fin) {
    super_block block;
    for (int i = 0; i < BLOCK_SIZE; i ++)
        block.inode_bitmap[i] = (fin.get() != '\0');
    for (int i = 0; i < BLOCK_SIZE; i ++)
        block.block_bitmap[i] = (fin.get() != '\0');
    return block;
}

inode readInode(ifstream &fin) {
    inode block;
    block.i_id = getInt(fin);
    block.i_mode = getInt(fin);
    block.i_file_size = getInt(fin);
    for (int i = 0; i < FILE_MAX_BLOCKS; i ++)
        block.i_blocks[i] = getInt(fin);
    for (int i = 0; i < INODE_PLACEHOLDER_SIZE; i ++)
        block.i_placeholder[i] = getInt(fin);
    return block;
}

file_block readFileBlock(ifstream &fin) {
    file_block block;
    for (int i = 0; i < BLOCK_SIZE; i ++)
        block.data[i] = fin.get();
    return block;
}

dir_block readDirBlock(ifstream &fin) {
    dir_block block;
    for (int i = 0; i < MAX_DIR_ENTRIES_IN_BLOCK; i ++) {
        for (int j = 0; j < MAX_FILENAME_SIZE; j ++)
            block.dirs[i].name[j] = fin.get();
        block.dirs[i].inode_id = getInt(fin);
    }
    return block;
}

disk_file readDisk(ifstream &fin) {
    disk_file disk;

    disk.superBlockPointer = new super_block;
    *disk.superBlockPointer = readSuperBlock(fin);

    disk.inodePointer = new inode[4096];
    for (int i = 0; i < 4096; i ++)
        disk.inodePointer[i] = readInode(fin);
    
    disk.dirBlockPointer = new dir_block[4096];
    disk.fileBlockPointer = new file_block[4096];
    for (int i = 0; i < 4096; i ++) {
        if (disk.inodePointer[i].i_mode == 1)
            disk.dirBlockPointer[i] = readDirBlock(fin);
        else
            disk.fileBlockPointer[i] = readFileBlock(fin);
        disk.inodeName[i] = "";
    }

    memset(disk.blockType, 0, sizeof(disk.blockType));
    for (int i = 0; i < 4096; i ++) {
        if (disk.superBlockPointer->inode_bitmap[i]) {
            for (int j = 0; j < FILE_MAX_BLOCKS; j ++)
                disk.blockType[disk.inodePointer[i].i_blocks[j]] = disk.inodePointer[i].i_mode;
            if (disk.inodePointer[i].i_mode == 1) {
                for (int j = 0; j < MAX_DIR_ENTRIES_IN_BLOCK; j ++)
                    if (strlen(disk.dirBlockPointer[disk.inodePointer[i].i_blocks[0]].dirs[j].name)
                        && strcmp(disk.dirBlockPointer[disk.inodePointer[i].i_blocks[0]].dirs[j].name, ".")
                        && strcmp(disk.dirBlockPointer[disk.inodePointer[i].i_blocks[0]].dirs[j].name, ".."))
                    disk.inodeName[disk.dirBlockPointer[disk.inodePointer[i].i_blocks[0]].dirs[j].inode_id] = disk.dirBlockPointer[disk.inodePointer[i].i_blocks[0]].dirs[j].name;
            }
        }
    }
    return disk;
}


bool checkDisk(const string &fileName) {
    ifstream fin(fileName.c_str(), ios::binary);
    bool fileExists = fin.good();
    fin.close();
    return fileExists;
}

bool format(const string &fileName) {
    disk_file disk;
    memset(disk.blockType, 0x01, sizeof(disk.blockType));

    disk.superBlockPointer = new super_block;
    memset(disk.superBlockPointer, 0, sizeof(super_block));
    disk.superBlockPointer->block_bitmap[0] = 1;
    disk.superBlockPointer->inode_bitmap[0] = 1;

    disk.inodePointer = new inode[4096];
    inode inodeBlock[4096];
    memset(disk.inodePointer, 0, sizeof(inode) * 4096);

    disk.inodePointer[0].i_id = 0;
    disk.inodePointer[0].i_mode = 1;       //dir
    disk.inodePointer[0].i_file_size = 256;
    disk.inodePointer[0].i_blocks[0] = 0;

    for (int i = 1; i < 4096; i ++) {
        disk.inodePointer[i].i_id = i;
        disk.inodePointer[i].i_mode = 0;
        disk.inodePointer[i].i_file_size = 0;
    }

    disk.dirBlockPointer = new dir_block[4096];
    if (disk.dirBlockPointer == NULL) {
        cout << "Allocating Memory Failed" << endl;
        return false;
    }
    memset(disk.dirBlockPointer, 0, sizeof(dir_block) * 4096);
    strcpy(disk.dirBlockPointer[0].dirs[0].name, ".");
    disk.dirBlockPointer[0].dirs[0].inode_id = 0;

    ofstream fout(fileName.c_str(), ios::binary);
    if (fout.bad()) {
        cout << "Creating virtualdisk file failed, please check your disk space and permission" << endl;
        fout.close();
        return false;
    }
    writeDisk(fout, disk);
    
    fout.close();

    delete disk.superBlockPointer;
    delete [] disk.inodePointer;
    delete [] disk.dirBlockPointer;

    return true;
}

