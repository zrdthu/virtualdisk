#include"operation.h"
#include<iostream>
#include <cstring>
using namespace std;

/***********
 * dst: Ŀ��λ��·��
 * curInode:
 * disk:
 * return: �Ƿ�ɹ�? 
 *      succeeded: 1
 *      failed:  errorCode
 **********/

int echo(string& str, const string&dst,int cur_inode, disk_file&disk){
	if (str.size() && str[0] == '\"')
		str.erase(str.begin());
	if (str.size() && str[str.size() - 1] == '\"')
		str.erase(str.end()-1);
	string parentDir = "";
	if (dst.length() && dst[0] == '/')
		parentDir += "/";
	vector<string> dstSplit = splitString(dst, "/");
	if (!dstSplit.size())  //??????����-2��
		return -2;
	for (unsigned int i = 0; i < dstSplit.size() - 1; i ++)
		parentDir += dstSplit[i] + "/";
	string fileName = dstSplit[dstSplit.size() - 1];

	int cdReturn = cd(parentDir, cur_inode, disk);                           //���е���Ŀ¼ȥ
	if (cdReturn <= 0)                                                      //ʧ��
		return cdReturn;

	if (str.size() >= BLOCK_SIZE)
		return -5;
	int i;
	for (i = 0; i < MAX_DIR_ENTRIES_IN_BLOCK; i ++) {
		string tmp = disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[i].name;
		if (tmp == fileName)   //
			break;
	}
    if(i!=MAX_DIR_ENTRIES_IN_BLOCK){//����ͬ��Ŀ¼�����ļ�
		int fileNode=disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[i].inode_id;
		if(disk.inodePointer[fileNode].i_mode)//��·��ΪĿ¼
			return -4;
		disk.inodePointer[fileNode].i_file_size=str.length();
		disk.inodePointer[fileNode].i_modification_time = time(NULL);
		for(int k=0;k<str.length();k++)disk.fileBlockPointer[disk.inodePointer[fileNode].i_blocks[0]].data[k]=str[k];//����ԭ�ļ�
	}
	else{
		int newDirPos;
		for (newDirPos = 0; newDirPos < MAX_DIR_ENTRIES_IN_BLOCK; newDirPos ++)
			if (!strlen(disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[newDirPos].name))       //�ҵ�һ����λ��
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


		//��ʼ�������ļ�
		disk.inodeName[newInodeNum]=fileName;
		disk.blockType[newBlockNum]=0;

		strcpy(disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[newDirPos].name, fileName.c_str());
		disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[newDirPos].inode_id = newInodeNum;

		disk.superBlockPointer->inode_bitmap[newInodeNum] = 1;
		disk.superBlockPointer->block_bitmap[newBlockNum] = 1;

		disk.inodePointer[newInodeNum].i_mode = 0;
		disk.inodePointer[newInodeNum].i_file_size = str.length();
		disk.inodePointer[newInodeNum].i_blocks[0] = newBlockNum;
		disk.inodePointer[newInodeNum].i_creation_time = time(NULL);
		disk.inodePointer[newInodeNum].i_modification_time = time(NULL);
		
		strcpy(disk.fileBlockPointer[newBlockNum].data, str.c_str());

	}
	return 1;
}