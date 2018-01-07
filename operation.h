#ifndef _OPERATION_H
#define _OPERATION_H

#include <string>
#include<vector>
#include "disk.h"

std::string getTime(int time);
std::vector<std::string> splitString(const std::string &src, const std::string &splitor) ;
void errorAlert(const std::string &cmd, const std::string &path, int errorCode);
void pwd(int curInode, disk_file &disk);
int cd(const std::string &dst, int &curInode, disk_file &disk);
int mkdir(const std::string &dst, int curInode, disk_file &disk);
int ls(const std::string &dst, const std::string &opt, int curInode, disk_file &disk);
int rmdir(const std::string&dst,int cur_inode, disk_file&disk);
int cat(const std::string&dst,int cur_inode, disk_file&disk);
int rm(const std::string&dst,int cur_inode, disk_file&disk);
int echo(std::string&str, const std::string&dst,int cur_inode, disk_file&disk);
int rn(const std::string&, const std::string&, int, disk_file&);
int mv(const std::string&, const std::string&, int, disk_file&);
int cf(const std::string&, const std::string&, const std::string&, int, disk_file&);

void help(const std::string &cmd);
#endif