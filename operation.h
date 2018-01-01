#ifndef _OPERATION_H
#define _OPERATION_H

#include <string>
#include "disk.h"

void errorAlert(const std::string &cmd, const std::string &path, int errorCode);
void pwd(int curInode, disk_file &disk);
int cd(const std::string &dst, int &curInode, disk_file &disk);
int mkdir(const std::string &dst, int curInode, disk_file &disk);
int ls(const std::string &dst, int curInode, disk_file &disk);

#endif