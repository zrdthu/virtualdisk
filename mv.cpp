#include"operation.h"
#include<iostream>
#include <cstring>
using namespace std;

/***********
 * dst:初始位置路径
 * path:目标移动路径
 * curInode:
 * disk:
 * return: 是否成功 
 *      succeeded: 1 
 *      failed:  errorCode
 **********/

int mv(const string&dst,const string&path,int cur_inode, disk_file&disk){
	int finalNode=cur_inode;//保存目标移动路径对应的目录
	string parentDir = "";
	if (dst.length() && dst[0] == '/')
		parentDir += "/";
	vector<string> dstSplit = splitString(dst, "/");
	if (!dstSplit.size())  //??????返回-2？
		return -2;
	for (unsigned int i = 0; i < dstSplit.size() - 1; i ++)
		parentDir += dstSplit[i] + "/";
	string fileName = dstSplit[dstSplit.size() - 1];

	if(parentDir.length()){
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
	if (i == MAX_DIR_ENTRIES_IN_BLOCK)                                      //没有找到该文件/目录
		return -1;

	int targeNode=disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[i].inode_id;//文件或目录序号

	if(!targeNode){cout<<"root directory cannot move"<<endl;return 1;}
	if(i<2){//目录形如 ../或者./
		cur_inode=disk.dirBlockPointer[disk.inodePointer[targeNode].i_blocks[0]].dirs[1].inode_id;
		for(int k=2;k<MAX_DIR_ENTRIES_IN_BLOCK;k++){
			string temp=disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[k].name;
			if(temp==disk.inodeName[targeNode]){i=k;break;}
		}
	}

	for(int k=0;k<MAX_FILENAME_SIZE;k++)
		disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[i].name[k]='\0';//删除上级目录的相关信息


	cd(path, finalNode, disk);                           //先切到父目录去


	//找新的父级目录下未使用的子目录
	int newDirPos;
	for (newDirPos = 0; newDirPos < MAX_DIR_ENTRIES_IN_BLOCK; newDirPos ++)
		if (!strlen(disk.dirBlockPointer[disk.inodePointer[finalNode].i_blocks[0]].dirs[newDirPos].name))       //找到一个空位置
			break;
	if (newDirPos == MAX_DIR_ENTRIES_IN_BLOCK)                              //没找到
		return -5;

	//开始移动
	strcpy(disk.dirBlockPointer[disk.inodePointer[finalNode].i_blocks[0]].dirs[newDirPos].name, disk.inodeName[targeNode].c_str());
	disk.dirBlockPointer[disk.inodePointer[finalNode].i_blocks[0]].dirs[newDirPos].inode_id = targeNode;
    if(disk.inodePointer[targeNode].i_mode)//移动的是目录
		disk.dirBlockPointer[disk.inodePointer[targeNode].i_blocks[0]].dirs[1].inode_id = finalNode;

	return 1;
}

