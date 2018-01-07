#include"operation.h"
#include "disk.h"
#include<iostream>
#include <ctime>
using namespace std;

/***********
 * dst: 目锟斤拷位锟斤拷路锟斤拷
 * curInode:
 * disk:
 * return: 锟角凤拷晒锟� 
 *      succeeded: 1
 *      failed:  errorCode
 **********/


int rm(const string&dst,int cur_inode, disk_file &disk){
	string parentDir = "";
	if (dst.length() && dst[0] == '/')
		parentDir += "/";
	vector<string> dstSplit = splitString(dst, "/");
	if (!dstSplit.size())  //??????锟斤拷锟斤拷-2锟斤拷
		return -2;
	for (unsigned int i = 0; i < dstSplit.size() - 1; i ++)
		parentDir += dstSplit[i] + "/";
	string fileName = dstSplit[dstSplit.size() - 1];

	int cdReturn = cd(parentDir, cur_inode, disk);                           //锟斤拷锟叫碉拷锟斤拷目录去
	if (cdReturn <= 0)                                                      //失锟斤拷
		return cdReturn;

	int i;
	for (i = 0; i < MAX_DIR_ENTRIES_IN_BLOCK; i ++) {
		string tmp = disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[i].name;
		if (tmp == fileName)
			break;
	}
	if (i == MAX_DIR_ENTRIES_IN_BLOCK)                                      //没锟斤拷锟揭碉拷锟斤拷锟侥硷拷
	return -1;

	int fileNode=disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[i].inode_id;
	if(disk.inodePointer[fileNode].i_mode)//锟斤拷路锟斤拷为目录
		return -4;

	//锟斤拷始删锟斤拷
	for(int k=0;k<MAX_FILENAME_SIZE;k++)disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[i].name[k]='\0';
	disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[i].inode_id=-1;   //删锟斤拷锟斤拷锟斤拷目录锟斤拷锟斤拷锟斤拷锟较拷锟斤拷锟斤拷锟斤拷帧锟斤拷锟斤拷锟斤拷锟斤拷指锟斤拷-1

	disk.superBlockPointer->block_bitmap[fileNode]=0;
	disk.superBlockPointer->inode_bitmap[disk.inodePointer[fileNode].i_blocks[0]]=0;
	disk.inodePointer[cur_inode].i_modification_time = time(NULL);
	
	return 1;
}