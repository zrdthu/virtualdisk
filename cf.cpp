#include"operation.h"
#include<iostream>
#include <cstring>
#include <ctime>
using namespace std;


/***********
 * dst:原文件位置路径
 * path:副本路径
 * name:副本文件名
 * curInode:
 * disk:
 * return: 是否成功 
 *      succeeded: 1 
 *      failed:  errorCode
 **********/

int cf(const string&dst,const string&path,const string&name,int cur_inode, disk_file&disk){
	int targeNode=cur_inode;
	string parentDir = "";
	if (dst.length() && dst[0] == '/')
		parentDir += "/";
	vector<string> dstSplit = splitString(dst, "/");
	if (!dstSplit.size())  //??????返回-2？
		return -2;
	for (unsigned int i = 0; i < dstSplit.size() - 1; i ++)
		parentDir += dstSplit[i] + "/";
	string fileName = dstSplit[dstSplit.size() - 1];

	int cdReturn = cd(parentDir, cur_inode, disk);                           //先切到父目录去
	if (cdReturn <= 0)                                                      //失败
		return cdReturn;

	int i;
	for (i = 0; i < MAX_DIR_ENTRIES_IN_BLOCK; i ++) {
		string tmp = disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[i].name;
		if (tmp == fileName)
			break;
	}
	if (i == MAX_DIR_ENTRIES_IN_BLOCK)                                      //没有找到该文件
		return -1;

	int fileNode=disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[i].inode_id;
	int filesize=disk.inodePointer[fileNode].i_file_size;
	if(disk.inodePointer[fileNode].i_mode)//该路径为目录
		return -4;



	 cd(path, targeNode, disk);                           //先切到父目录去



	int newDirPos;
	for (newDirPos = 0; newDirPos < MAX_DIR_ENTRIES_IN_BLOCK; newDirPos ++)
		if (!strlen(disk.dirBlockPointer[disk.inodePointer[targeNode].i_blocks[0]].dirs[newDirPos].name))       //找到一个空位置
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

	//开始创建新文件
	disk.inodeName[newInodeNum]=name;
	disk.blockType[newBlockNum]=0;

	strcpy(disk.dirBlockPointer[disk.inodePointer[targeNode].i_blocks[0]].dirs[newDirPos].name, name.c_str());
	disk.dirBlockPointer[disk.inodePointer[targeNode].i_blocks[0]].dirs[newDirPos].inode_id = newInodeNum;

	disk.superBlockPointer->inode_bitmap[newInodeNum] = 1;
	disk.superBlockPointer->block_bitmap[newBlockNum] = 1;

	disk.inodePointer[newInodeNum].i_mode = 0;
	disk.inodePointer[newInodeNum].i_file_size = disk.inodePointer[fileNode].i_file_size;
	disk.inodePointer[newInodeNum].i_blocks[0] = newBlockNum;
	disk.inodePointer[newInodeNum].i_creation_time = time(NULL);
	disk.inodePointer[newInodeNum].i_modification_time = time(NULL);
	


	strcpy(disk.fileBlockPointer[newBlockNum].data, disk.fileBlockPointer[disk.inodePointer[fileNode].i_blocks[0]].data);
	return 1;
}