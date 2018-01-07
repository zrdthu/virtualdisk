#include"operation.h"
#include<iostream>
#include <cstring>
using namespace std;

/***********
 * dst: Ŀ��λ��·��
 * curInode:�ɹ�������Ϊ����Ŀ¼�������򱣳ֲ���
 * disk:
 * return: �Ƿ�ɹ� 
 *      succeeded: 1
 *      failed:  errorCode
 **********/

bool Leaf_node(disk_file&disk,int BlockNum){
	bool state=true;
	if(!disk.blockType[BlockNum])return state;//���Ϊ���ݿ��ΪҶ�ӽ��
	for(int i=2;i<MAX_DIR_ENTRIES_IN_BLOCK;i++){
		if(strlen(disk.dirBlockPointer[BlockNum].dirs[i].name))
		{state=false;break;}
	}
    return state;
}

bool searchCurNode(int cur_inode,int targeNode, disk_file&disk){
	if(targeNode==cur_inode)//���Ϊ��ǰĿ¼���ҵ���
		return true;
	else if(Leaf_node(disk,disk.inodePointer[targeNode].i_blocks[0]))//��㲻Ϊ��ǰĿ¼����ΪҶ�ӽ��
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

void dirdelete(int pre_node,int pos,int targeNode, disk_file&disk){//pos��ʾ���ϼ�Ŀ¼�е����
	if(Leaf_node(disk,disk.inodePointer[targeNode].i_blocks[0])){//��ɾ��Ҷ�ӽ��Ϊ���ݿ��ΪĿ¼��Ҷ�ӽ��
		disk.superBlockPointer->block_bitmap[disk.inodePointer[targeNode].i_blocks[0]]=0;		
		disk.superBlockPointer->inode_bitmap[targeNode]=0;
		for(int k=0;k<MAX_FILENAME_SIZE;k++)
			disk.dirBlockPointer[disk.inodePointer[pre_node].i_blocks[0]].dirs[pos].name[k]='\0';//ɾ���ϼ�Ŀ¼�������Ϣ
	}
	else {//Ŀ¼�鲻ΪҶ�ӽڵ㣬�ݹ�ɾ��
		for(int i=2;i<MAX_DIR_ENTRIES_IN_BLOCK;i++)
			if(strlen(disk.dirBlockPointer[disk.inodePointer[targeNode].i_blocks[0]].dirs[i].name))
			dirdelete(targeNode,i,disk.dirBlockPointer[disk.inodePointer[targeNode].i_blocks[0]].dirs[i].inode_id,disk);
		dirdelete(pre_node,pos,targeNode,disk);
	}
}

int rmdir(const string&dst,int cur_inode, disk_file&disk){//ɾ��Ŀ¼��Ŀ¼�µ��ļ�
	string parentDir = "";int oriNode=cur_inode;
	if (dst.length() && dst[0] == '/')
		parentDir += "/";
	vector<string> dstSplit = splitString(dst, "/");
	if (!dstSplit.size())  //??????����-2��
		return -2;
	for (unsigned int i = 0; i < dstSplit.size() - 1; i ++)
		parentDir += dstSplit[i] + "/";
	string dirName = dstSplit[dstSplit.size() - 1];
	if(parentDir.length()){
		int cdReturn = cd(parentDir, cur_inode, disk);                           //���е���Ŀ¼ȥ
		if (cdReturn <= 0)                                                      //ʧ��
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
	if (i == MAX_DIR_ENTRIES_IN_BLOCK)                                      //û���ҵ���Ŀ¼
	return -1;
	
	int dirNode=disk.dirBlockPointer[disk.inodePointer[cur_inode].i_blocks[0]].dirs[i].inode_id;

	if(searchCurNode(oriNode,dirNode,disk))//���ɾ��Ŀ¼�º��е�ǰĿ¼��ɾ
	{cout<<"current dirctionay is non-empty"<<endl;return 1;}

	if(!disk.inodePointer[dirNode].i_mode)return -3;//��·��Ϊ�ļ�
	dirdelete(cur_inode,i,dirNode,disk);

	return 1;
}

