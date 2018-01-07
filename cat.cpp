#include"operation.h"
#include<iostream>
using namespace std;

/***********
 * dst: Ŀ��λ��·��
 * curInode:
 * disk:
 * return: �Ƿ�ɹ� 
 *      succeeded: 1
 *      failed:  errorCode
 **********/

int cat(const string&dst,int cur_inode, disk_file&disk){

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

	int i;
	for (i = 0; i < MAX_DIR_ENTRIES_IN_BLOCK; i ++) {
		string tmp = disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[i].name;
		if (tmp == fileName)
			break;
	}
	if (i == MAX_DIR_ENTRIES_IN_BLOCK)                                      //û���ҵ����ļ�
	return -1;

	int fileNode=disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[i].inode_id;
	int filesize=disk.inodePointer[fileNode].i_file_size;
	if(disk.inodePointer[fileNode].i_mode)//��·��ΪĿ¼
		return -4;

	for(int k=0;k<filesize;k++)
		cout<<disk.fileBlockPointer[disk.inodePointer[fileNode].i_blocks[0]].data[k];
	cout<<endl;

	return 1;
}