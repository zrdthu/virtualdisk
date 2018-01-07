#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <iomanip>
#include <ctime>
#include "disk.h"
using namespace std;

string getTime(int time) {
    time_t tt = (time_t)time;
    struct tm *tmStruct = localtime(&tt);
    char buff[80];
    strftime(buff, 80, "%Y-%m-%d %H:%M:%S", tmStruct);
    return (string)buff;
}

vector<string> splitString(const string &src, const string &splitor) {
    string::size_type pos1, pos2;
    vector<string> v;
    pos2 = src.find(splitor);
    pos1 = 0;
    while(string::npos != pos2) {
        if (src.substr(pos1, pos2 - pos1).length())
            v.push_back(src.substr(pos1, pos2 - pos1));
        pos1 = pos2 + splitor.size();
        pos2 = src.find(splitor, pos1);
    }
    if(pos1 != src.length())
        v.push_back(src.substr(pos1));
    return v;
}

void errorAlert(const string &cmd, const string &path, int errorCode) {
    if (errorCode > 0)
        return;
    string msg;
    switch (errorCode) {
        case -1: msg = path + " does not exist";        break;
        case -2: msg = path + " already exists";        break;
        case -3: msg = path + " is not a directory";    break;
        case -4: msg = path + " is not a file";         break;
        case -5: msg = "Not enough space in" + path;    break;
        case -6: msg = "Not enough space in disk";      break;
        case -7: msg = "System out of memory";          break;
        default: msg = "Unknown error";                 break;
    }
    cout << cmd << ": " << msg << endl;
}

void pwd(int curInode, disk_file &disk) {
    //vector<string> path;
    string path = "";
    while (curInode) {
        path = "/" + disk.inodeName[curInode] + path;
        curInode = disk.dirBlockPointer[disk.inodePointer[curInode].i_blocks[0]].dirs[1].inode_id;         //找到其父目录的inode编号
    }
    if (!path.size())
        path = "/";
    cout << path << endl;
}

/***********
 * dst: 目标位置路径
 * curInode:
 * disk:
 * return: 是否成功 
 *      succeeded: 1
 *      failed:  errorCode
 **********/

int cd(const string &dst, int &curInode, disk_file &disk) {                //返回是否成功
    int oriInode = curInode;
    if (dst.length() && dst[0] == '/')
        curInode = 0;
    vector<string> dstPath = splitString(dst, "/");
    unsigned int i;
    for (i = 0; i < dstPath.size(); i ++) {
        int j;
        for (j = 0; j < MAX_DIR_ENTRIES_IN_BLOCK; j ++) {
            string tmp = disk.dirBlockPointer[disk.inodePointer[curInode].i_blocks[0]].dirs[j].name;
            if (tmp == dstPath[i]) {                                        //找到了，更新当前指向，进入下一层
                curInode = disk.dirBlockPointer[disk.inodePointer[curInode].i_blocks[0]].dirs[j].inode_id;
                if (disk.inodePointer[curInode].i_mode != 1) {              //然而目标不是目录
                    curInode = oriInode;
                    return -3;
                }
                break;
            }
        }
        if (j == MAX_DIR_ENTRIES_IN_BLOCK)                                  //没找到
            break;
    }
    if (i != dstPath.size()) {                                              //没找到
        curInode = oriInode;                                                //把当前目录复原
        return -1;
    }
    return 1;
}

/***********
 * dst: 目标位置路径
 * curInode:
 * disk:
 * return: 是否成功 
 *      succeeded: 1
 *      failed:  errorCode
 **********/

int mkdir(const string &dst, int curInode, disk_file &disk) {
    string parentDir = "";
    if (dst.length() && dst[0] == '/')
        parentDir += "/";
    vector<string> dstSplit = splitString(dst, "/");
    if (!dstSplit.size())
        return -2;
    for (unsigned int i = 0; i < dstSplit.size() - 1; i ++)
        parentDir += dstSplit[i] + "/";
    string targetName = dstSplit[dstSplit.size() - 1];

    int cdReturn = cd(parentDir, curInode, disk);                           //先切到父目录去
    if (cdReturn <= 0)                                                      //失败
        return cdReturn;

    int i;
    for (i = 0; i < MAX_DIR_ENTRIES_IN_BLOCK; i ++) {
        string tmp = disk.dirBlockPointer[disk.inodePointer[curInode].i_blocks[0]].dirs[i].name;
        if (tmp == targetName)
            break;
    }
    if (i != MAX_DIR_ENTRIES_IN_BLOCK)                                      //已存在同名目录/文件
        return -2;

    int newDirPos;
    for (newDirPos = 0; newDirPos < MAX_DIR_ENTRIES_IN_BLOCK; newDirPos ++)
        if (!strlen(disk.dirBlockPointer[disk.inodePointer[curInode].i_blocks[0]].dirs[newDirPos].name))       //找到一个空位置
            break;
    if (newDirPos == MAX_DIR_ENTRIES_IN_BLOCK)                              //没找到
        return -5;

    int newInodeNum, newBlockNum;
    for (newInodeNum = 0; newInodeNum < BLOCK_SIZE; newInodeNum ++)
        if (!disk.superBlockPointer->inode_bitmap[newInodeNum])
            break;
    for (newBlockNum = 0; newBlockNum < BLOCK_SIZE; newBlockNum ++)
        if (!disk.superBlockPointer->block_bitmap[newBlockNum])
            break;
    if (newInodeNum == BLOCK_SIZE || newBlockNum == BLOCK_SIZE)
        return -6;
  
    //一切就绪，开始写新目录
    disk.inodeName[newInodeNum] = targetName;
    disk.blockType[newBlockNum] = 1;

    strcpy(disk.dirBlockPointer[disk.inodePointer[curInode].i_blocks[0]].dirs[newDirPos].name, targetName.c_str());
    disk.dirBlockPointer[disk.inodePointer[curInode].i_blocks[0]].dirs[newDirPos].inode_id = newInodeNum;

    disk.superBlockPointer->inode_bitmap[newInodeNum] = 1;
    disk.superBlockPointer->block_bitmap[newBlockNum] = 1;

    disk.inodePointer[newInodeNum].i_mode = 1;
    disk.inodePointer[newInodeNum].i_file_size = 256;
    disk.inodePointer[newInodeNum].i_blocks[0] = newBlockNum;
    disk.inodePointer[newInodeNum].i_creation_time = time(NULL);
    disk.inodePointer[newInodeNum].i_modification_time = time(NULL);

    strcpy(disk.dirBlockPointer[newBlockNum].dirs[0].name, ".");
    disk.dirBlockPointer[newBlockNum].dirs[0].inode_id = newInodeNum;
    strcpy(disk.dirBlockPointer[newBlockNum].dirs[1].name, "..");
    disk.dirBlockPointer[newBlockNum].dirs[1].inode_id = curInode;

    return 1;
}

int ls(const string &dst, const string &opt, int curInode, disk_file &disk) {
    int cdReturn = cd(dst, curInode, disk);
    if (cdReturn == -3) {                                       //目标是个文件
        if (opt.find('l') != string::npos) {
            int tmpInode = curInode;
            string parentDir = "", targetName = "";
            if (dst[0] == '/')
                parentDir += "/";
            vector<string> v = splitString(dst, "/");
            for (int i = 0; i < v.size() - 1; i ++)
                parentDir += (v[i] + "/");
            targetName = v[v.size() - 1];
            cd(parentDir, tmpInode, disk);
            int targetInode;
            for (int i = 0; i < MAX_DIR_ENTRIES_IN_BLOCK; i ++)
                if (targetName == (string)disk.dirBlockPointer[disk.inodePointer[tmpInode].i_blocks[0]].dirs[i].name) {
                    targetInode = disk.dirBlockPointer[disk.inodePointer[tmpInode].i_blocks[0]].dirs[i].inode_id;
                    break;
                }

            cout << "Type    Size      Creation Time       Modification Time    Name" << endl;
            string isDir = (disk.inodePointer[targetInode].i_mode ? "d" : "f");
            cout << setw(2) << isDir;
            cout << setw(10) << disk.inodePointer[targetInode].i_file_size;
            cout << setw(22) << getTime(disk.inodePointer[targetInode].i_creation_time);
            cout << setw(22) << getTime(disk.inodePointer[targetInode].i_modification_time);
            cout << "   ";
        }

        cout << dst << endl;                                    //原样显示即可
        return 1;
    }
    if (cdReturn <= 0)                                          //切到目标目录失败
        return cdReturn;
    
    if (opt.find('l') != string::npos) {
        cout << "Type    Size      Creation Time       Modification Time    Name" << endl;
    }
    for (int i = 0; i < MAX_DIR_ENTRIES_IN_BLOCK; i ++) {
        string name = disk.dirBlockPointer[disk.inodePointer[curInode].i_blocks[0]].dirs[i].name;
        if (name.size()) {
            int subTgtInode = disk.dirBlockPointer[disk.inodePointer[curInode].i_blocks[0]].dirs[i].inode_id;
            if (opt.find('a') == string::npos && name[0] == '.')
                continue;
            string endByte = " ";
            if (opt.find('l') != string::npos) {
                string isDir = (disk.inodePointer[subTgtInode].i_mode ? "d" : "f");
                cout << setw(2) << isDir;
                cout << setw(10) << disk.inodePointer[subTgtInode].i_file_size;
                cout << setw(22) << getTime(disk.inodePointer[subTgtInode].i_creation_time);
                cout << setw(22) << getTime(disk.inodePointer[subTgtInode].i_modification_time);
                cout << "   ";
                endByte = "\n";
            }
            cout << name << endByte;
        }
    }
    cout << endl;
    return 1;
}

void help(const string &cmd) {
    if (cmd == "pwd") {
        cout << "Usage: pwd" << endl;
        cout << "Display the absolute path of current directory." << endl;
    }
    else if (cmd == "mkdir") {
        cout << "Usage: mkdir $path" << endl;
        cout << "Make a new directory at $path." << endl;
    }
    else if (cmd == "cd") {
        cout << "Usage: cd [$path]" << endl;
        cout << "Default for $path: /" << endl;
        cout << "Change the current directory to $path" << endl;
    }
    else if (cmd == "ls") {
        cout << "Usage: ls [$option] [$path]" << endl;
        cout << "Options:" << endl;
        cout << "   -a: display all items, including hidden ones" << endl;
        cout << "   -l: display details in a table" << endl;
        cout << "Default for $option: none" << endl;
        cout << "Default for $path: ." << endl;
    }
    else if (cmd == "rmdir") {
        cout << "Usage: rmdir $path" << endl;
        cout << "Remove directory $path and all its contents recursively" << endl;
    }
    else if (cmd == "echo") {
        cout << "Usage: echo $str $path" << endl;
        cout <<"Write $str into file $path, if $path already exists, overwrite file" << endl;
    }
    else if (cmd == "cat") {
        cout << "Usage: cat $path" << endl;
        cout << "Display the contents of file $path at the screen" << endl;
    }
    else if (cmd == "rm") {
        cout << "Usage: rm $path" << endl;
        cout << "Remove file $path" << endl;
    }
    else if (cmd == "rn") {
        cout << "Usage: rn $path $newname" << endl;
        cout << "Rename $path as $newname" << endl;
    }
    else if (cmd == "mv") {
        cout << "Usage: mv $source_path $target_path" << endl;
        cout << "Move $source_path and all its contents to $target_path recursively" << endl;
    }
    else if (cmd == "rf") {
        cout << "Usage: $source_path $target_path $new_filename" << endl;
        cout << "Move file $source_path to $target_path and rename it as $new_filename" << endl;
    }
    else if (cmd == "help") {
        cout << "Usage: help $cmd" << endl;
        cout << "Display the usage of $cmd" << endl;
    }
    else {
        cout << "No such command named " << cmd << endl;
    }
}