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
        curInode = disk.dirBlockPointer[disk.inodePointer[curInode].i_blocks[0]].dirs[1].inode_id;         //�ҵ��丸Ŀ¼��inode���
    }
    if (!path.size())
        path = "/";
    cout << path << endl;
}

/***********
 * dst: Ŀ��λ��·��
 * curInode:
 * disk:
 * return: �Ƿ�ɹ� 
 *      succeeded: 1
 *      failed:  errorCode
 **********/

int cd(const string &dst, int &curInode, disk_file &disk) {                //�����Ƿ�ɹ�
    int oriInode = curInode;
    if (dst.length() && dst[0] == '/')
        curInode = 0;
    vector<string> dstPath = splitString(dst, "/");
    unsigned int i;
    for (i = 0; i < dstPath.size(); i ++) {
        int j;
        for (j = 0; j < MAX_DIR_ENTRIES_IN_BLOCK; j ++) {
            string tmp = disk.dirBlockPointer[disk.inodePointer[curInode].i_blocks[0]].dirs[j].name;
            if (tmp == dstPath[i]) {                                        //�ҵ��ˣ����µ�ǰָ�򣬽�����һ��
                curInode = disk.dirBlockPointer[disk.inodePointer[curInode].i_blocks[0]].dirs[j].inode_id;
                if (disk.inodePointer[curInode].i_mode != 1) {              //Ȼ��Ŀ�겻��Ŀ¼
                    curInode = oriInode;
                    return -3;
                }
                break;
            }
        }
        if (j == MAX_DIR_ENTRIES_IN_BLOCK)                                  //û�ҵ�
            break;
    }
    if (i != dstPath.size()) {                                              //û�ҵ�
        curInode = oriInode;                                                //�ѵ�ǰĿ¼��ԭ
        return -1;
    }
    return 1;
}

/***********
 * dst: Ŀ��λ��·��
 * curInode:
 * disk:
 * return: �Ƿ�ɹ� 
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

    int cdReturn = cd(parentDir, curInode, disk);                           //���е���Ŀ¼ȥ
    if (cdReturn <= 0)                                                      //ʧ��
        return cdReturn;

    int i;
    for (i = 0; i < MAX_DIR_ENTRIES_IN_BLOCK; i ++) {
        string tmp = disk.dirBlockPointer[disk.inodePointer[curInode].i_blocks[0]].dirs[i].name;
        if (tmp == targetName)
            break;
    }
    if (i != MAX_DIR_ENTRIES_IN_BLOCK)                                      //�Ѵ���ͬ��Ŀ¼/�ļ�
        return -2;

    int newDirPos;
    for (newDirPos = 0; newDirPos < MAX_DIR_ENTRIES_IN_BLOCK; newDirPos ++)
        if (!strlen(disk.dirBlockPointer[disk.inodePointer[curInode].i_blocks[0]].dirs[newDirPos].name))       //�ҵ�һ����λ��
            break;
    if (newDirPos == MAX_DIR_ENTRIES_IN_BLOCK)                              //û�ҵ�
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
  
    //һ�о�������ʼд��Ŀ¼
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
    if (cdReturn == -3) {                                       //Ŀ���Ǹ��ļ�
        cout << dst << endl;                                    //ԭ����ʾ����
        return 1;
    }
    if (cdReturn <= 0)                                          //�е�Ŀ��Ŀ¼ʧ��
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
                string isDir = (disk.inodePointer[subTgtInode].i_mode ? "d" : "");
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
