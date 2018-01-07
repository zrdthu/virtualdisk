#include"operation.h"
#include<iostream>
#include <cstring>
#include <ctime>
using namespace std;

/***********
 * dst: 目标位置路径
 * curInode:
 * disk:
 * return: 是否成功 
 *      succeeded: 1
 *      failed:  errorCode
 **********/

int cat(const string&dst,int cur_inode, disk_file&disk){

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

	for(int k=0;k<filesize;k++)
		cout<<disk.fileBlockPointer[disk.inodePointer[fileNode].i_blocks[0]].data[k];
	cout<<endl;

	return 1;
}

/***********
 * dst: 目标位置路径
 * newName:
 * curInode:
 * disk:
 * return: 是否成功 
 *      succeeded: 1
 *      failed:  errorCode
 **********/
int rn(const string&dst,const string&newName,int cur_inode, disk_file&disk){
	string parentDir = "";
	if (dst.length() && dst[0] == '/')
		parentDir += "/";
	vector<string> dstSplit = splitString(dst, "/");
	if (!dstSplit.size())  //??????返回-2？
		return -2;
	for (unsigned int i = 0; i < dstSplit.size() - 1; i ++)
		parentDir += dstSplit[i] + "/";
	string fileName = dstSplit[dstSplit.size() - 1];

	if(parentDir.length()>0){
		int cdReturn = cd(parentDir, cur_inode, disk);                           //先切到父目录去
		if (cdReturn <= 0)                                                      //失败
			return cdReturn;
	}

	int i;
	for (i = 0; i < MAX_DIR_ENTRIES_IN_BLOCK; i ++) {
		string tmp = disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[i].name;
		if (tmp == fileName)
			break;
	}
	if (i == MAX_DIR_ENTRIES_IN_BLOCK)                                      //没有找到该文件
		return -1;

	int targeNode=disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[i].inode_id;//文件或目录序号

	if(!targeNode){cout<<"Root directory cannot alter"<<endl;return 1;}
	if(i<2){//目录形如 ../或者./
		cur_inode=disk.dirBlockPointer[disk.inodePointer[targeNode].i_blocks[0]].dirs[1].inode_id;
		for(int k=2;k<MAX_DIR_ENTRIES_IN_BLOCK;k++){
			string temp=disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[k].name;
			if(temp==disk.inodeName[targeNode]){i=k;break;}
		}
	}

	strcpy(disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[i].name,newName.c_str());
	disk.inodeName[targeNode]=newName;
	return 1;
}