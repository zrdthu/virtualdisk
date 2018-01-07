#include"operation.h"
#include<iostream>
#include <cstring>
using namespace std;

/***********
 * dst: 目标位置路径
 * curInode:成功则重置为父级目录，错误则保持不变
 * disk:
 * return: 是否成功 
 *      succeeded: 1
 *      failed:  errorCode
 **********/

bool Leaf_node(disk_file&disk,int BlockNum){
	bool state=true;
	if(!disk.blockType[BlockNum])return state;//如果为数据块必为叶子结点
	for(int i=2;i<MAX_DIR_ENTRIES_IN_BLOCK;i++){
		if(strlen(disk.dirBlockPointer[BlockNum].dirs[i].name))
		{state=false;break;}
	}
    return state;
}

bool searchCurNode(int cur_inode,int targeNode, disk_file&disk){
	if(targeNode==cur_inode)//结点为当前目录则找到了
		return true;
	else if(Leaf_node(disk,disk.inodePointer[targeNode].i_blocks[0]))//结点不为当前目录，但为叶子结点
		return false;
	else{
		bool state=false;
		for(int i=2;i<MAX_DIR_ENTRIES_IN_BLOCK;i++)
			if(strlen(disk.dirBlockPointer[disk.inodePointer[targeNode].i_blocks[0]].dirs[i].name)){
				int node=disk.dirBlockPointer[disk.inodePointer[targeNode].i_blocks[0]].dirs[i].inode_id;
				if(searchCurNode(cur_inode,node,disk)){state=true;break;}
			}
        return state;
	}
}

void dirdelete(int pre_node,int pos,int targeNode, disk_file&disk){//pos表示在上级目录中的序号
	if(Leaf_node(disk,disk.inodePointer[targeNode].i_blocks[0])){//待删除叶子结点为数据块或为目录树叶子结点
		disk.superBlockPointer->block_bitmap[disk.inodePointer[targeNode].i_blocks[0]]=0;		
		disk.superBlockPointer->inode_bitmap[targeNode]=0;
		for(int k=0;k<MAX_FILENAME_SIZE;k++)
			disk.dirBlockPointer[disk.inodePointer[pre_node].i_blocks[0]].dirs[pos].name[k]='\0';//删除上级目录中相关信息
	}
	else {//目录块不为叶子节点，递归删除
		for(int i=2;i<MAX_DIR_ENTRIES_IN_BLOCK;i++)
			if(strlen(disk.dirBlockPointer[disk.inodePointer[targeNode].i_blocks[0]].dirs[i].name))
			dirdelete(targeNode,i,disk.dirBlockPointer[disk.inodePointer[targeNode].i_blocks[0]].dirs[i].inode_id,disk);
		dirdelete(pre_node,pos,targeNode,disk);
	}
}

int rmdir(const string&dst,int cur_inode, disk_file&disk){//删除目录及目录下的文件
	string parentDir = "";int oriNode=cur_inode;
	if (dst.length() && dst[0] == '/')
		parentDir += "/";
	vector<string> dstSplit = splitString(dst, "/");
	if (!dstSplit.size())  //??????返回-2？
		return -2;
	for (unsigned int i = 0; i < dstSplit.size() - 1; i ++)
		parentDir += dstSplit[i] + "/";
	string dirName = dstSplit[dstSplit.size() - 1];
	if(parentDir.length()){
		int cdReturn = cd(parentDir, cur_inode, disk);                           //先切到父目录去
		if (cdReturn <= 0)                                                      //失败
			return cdReturn;
	}
	else{
		cout<<"current dictionary is non-empty"<<endl;
		return 1;
	}

	int i;
	for (i = 2; i < MAX_DIR_ENTRIES_IN_BLOCK; i ++) {
		string tmp = disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[i].name;
		if (tmp == dirName)
			break;
	}
	if (i == MAX_DIR_ENTRIES_IN_BLOCK)                                      //没有找到该目录
	return -1;
	
	int dirNode=disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[i].inode_id;

	if(searchCurNode(oriNode,dirNode,disk))//如果删除目录下含有当前目录则不删
	{cout<<"current dirctionay is non-empty"<<endl;return 1;}

	if(!disk.inodePointer[dirNode].i_mode)return -3;//该路径为文件
	dirdelete(cur_inode,i,dirNode,disk);

	return 1;
}

