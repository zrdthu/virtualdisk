#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>

#include "disk.h"
#include "operation.h"

using namespace std;

bool isLegal(const string &path) {
    for (int i = 0; i < path.size(); i ++)
        if ((path[i] >= '0' && path[i] <= '9')
            || (path[i] >= 'a' && path[i] <= 'z')
            ||(path[i] >= 'A' && path[i] <= 'Z')
            || path[i] == '_' || path[i] == '.' || path[i] == '/')
            ;
        else
            return false;
    return true;
}

int main(int argc, char **argv) {
    string diskFileName = "disk.virtualdisk";
    if (argc > 1)
        diskFileName = argv[1];
    bool fileOK = 1;
    if (!checkDisk(diskFileName)) {
        cout << "Disk file does not exist, creating an empty one" << endl;
        fileOK = format(diskFileName);
    }
    if (!fileOK)
        exit(0);
    


    ifstream fin(diskFileName.c_str(), ios::binary);

    streampos oriPos = fin.tellg();
    fin.seekg(0, ios::end);
    if (fin.tellg() != DISK_SIZE) {
        cout << "Disk file corrupted" << endl;
        exit(0);
    }
    fin.seekg(oriPos);
    disk_file disk = readDisk(fin);
    fin.close();
    int curInode = 0;

    int secs = time(NULL);
    cout << getTime(secs) << endl;

    string cmd, target, option;
    while(cmd != "exit" && cmd != "logout") {
        cout << ">> ";
        string rawCmd;
        getline(cin, rawCmd);
        vector<string> cmdSplit = splitString(rawCmd, " ");
        if (!cmdSplit.size())
            continue;
        cmd = cmdSplit[0];
        //cin >> cmd;
        if (cmd == "pwd")
            pwd(curInode, disk);
        else if (cmd == "cd") {
            if (cmdSplit.size() == 1)
                target = "/";
            else
                target = cmdSplit[1];
            if (isLegal(target))
                errorAlert(cmd, target, cd(target, curInode, disk));
            else
                cout << "Invalid path" << endl;
        }
        else if (cmd == "mkdir") {
            if (cmdSplit.size() < 2)
                help(cmd);
            else {
                target = cmdSplit[1];
                if (isLegal(target))
                    errorAlert(cmd, target, mkdir(target, curInode, disk));
                else
                    cout << "Invalid path" << endl;
            }
        }
        else if (cmd == "ls") {
            bool wrongUsage = 0;
            target = "";
            option = "";
            if (cmdSplit.size() == 1)
                target = ".";
            else if (cmdSplit.size() == 2) {
                if (cmdSplit[1][0] == '-') {
                    target = ".";
                    option = cmdSplit[1];
                }
                else {
                    target = cmdSplit[1];
                    option = "";
                }
            }
            else {
                target = "";
                option = "";
                for (unsigned int i = 1; i < cmdSplit.size(); i ++) {
                    if (cmdSplit[i][0] == '-')
                        option += cmdSplit[i];
                    else if (target.size() == 0)
                        target = cmdSplit[i];
                    else
                        wrongUsage = 1;
                }
            }
            if (wrongUsage) {
                help(cmd);
            }
            else {
                if (isLegal(target))
                    errorAlert(cmd, target, ls(target, option, curInode, disk));
                else
                    cout << "Invalid path" << endl;
            }
        }
		else if(cmd=="rmdir"){
			if (cmdSplit.size() < 2)
                help(cmd);
            else {
                target = cmdSplit[1];
                if (isLegal(target))
			        errorAlert(cmd,target,rmdir(target,curInode,disk));
                else
                    cout << "Invalid path" << endl;
            }
		}
		else if(cmd=="echo"){
            if (cmdSplit.size() < 3)
                help(cmd);
            else {
			    string str = cmdSplit[1];
                target = cmdSplit[2];
                if (isLegal(target))
			        errorAlert(cmd,target,echo(str,target,curInode,disk));
                else
                    cout << "Invalid path" << endl;
            }
		}
		else if(cmd=="cat"){
			if (cmdSplit.size() < 2)
                help(cmd);
            else {
                target = cmdSplit[1];
                if (isLegal(target))
			        errorAlert(cmd,target,cat(target,curInode,disk));
                else
                    cout << "Invalid path" << endl;
            }
		}
		else if(cmd=="rm"){
			if (cmdSplit.size() < 2)
                help(cmd);
            else {
                target = cmdSplit[1];
                if (isLegal(target))
			        errorAlert(cmd,target,rm(target,curInode,disk));
                else
                    cout << "Invalid path" << endl;
            }
		}
        else if(cmd=="rn"){
			if (cmdSplit.size() < 3)
                help(cmd);
            else {
                target = cmdSplit[1];
                string newName = cmdSplit[2];
                if (isLegal(target) && isLegal(newName))
			        errorAlert(cmd,target,rn(target, newName,curInode,disk));
                else
                    cout << "Invalid path" << endl;
            }
		}
        else if(cmd=="mv"){
			if (cmdSplit.size() < 3)
                help(cmd);
            else {
                target = cmdSplit[1];
                string dst = cmdSplit[2];
                if (isLegal(target) && isLegal(dst)) {
                    int tmpInode = curInode;
                    int cdReturn = cd(dst, tmpInode, disk);
                    if (cdReturn < 0)
                        errorAlert(cmd, dst, cdReturn);
                    else
			            errorAlert(cmd,target,mv(target, dst,curInode,disk));
                }
                else
                    cout << "Invalid path" << endl;
            }
		}
        else if(cmd=="cf"){
			if (cmdSplit.size() < 4)
                help(cmd);
            else {
                target = cmdSplit[1];
                string dst = cmdSplit[2], newName = cmdSplit[3];
                if (isLegal(target) && isLegal(dst) && isLegal(newName)) {
                    int tmpInode = curInode;
                    int cdReturn = cd(dst, tmpInode, disk);
                    if (cdReturn < 0)
                        errorAlert(cmd, dst, cdReturn);
                    else
			            errorAlert(cmd,target,cf(target, dst, newName,curInode,disk));
                }
                else
                    cout << "Invalid path" << endl;
            }
		}
        else if (cmd == "help") {
            if (cmdSplit.size() < 2)
                help(cmd);
            else
                help(cmdSplit[1]);
        }
        else if (cmd != "exit" && cmd != "logout")
            cout << cmd << ": Command not found" << endl;
    }

    ofstream fout(diskFileName.c_str(), ios::binary);
    writeDisk(fout, disk);
    fout.close();

    delete disk.superBlockPointer;
    delete [] disk.inodePointer;
    delete [] disk.dirBlockPointer;
    delete [] disk.fileBlockPointer;

    return 0;
}