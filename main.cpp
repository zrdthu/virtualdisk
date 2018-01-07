#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>

#include "disk.h"
#include "operation.h"

using namespace std;

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
            errorAlert(cmd, target, cd(target, curInode, disk));
        }
        else if (cmd == "mkdir") {
            if (cmdSplit.size() < 2)
                cout << "Usage: mkdir $path" << endl;
            else {
                target = cmdSplit[1];
                errorAlert(cmd, target, mkdir(target, curInode, disk));
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
                cout << "Usage: ls [option] [target]" << endl;
                cout << "    options:" << endl;
                cout << "    -a    display all items" << endl;
                cout << "    -l    display details" << endl;
            }
            else {
                errorAlert(cmd, target, ls(target, option, curInode, disk));
            }
        }
		else if(cmd=="rmdir"){
			if (cmdSplit.size() < 2)
                cout << "Usage: rmdir $path" << endl;
            else {
                target = cmdSplit[1];
			    errorAlert(cmd,target,rmdir(target,curInode,disk));
            }
		}
		else if(cmd=="echo"){
            if (cmdSplit.size() < 3)
                cout << "Usage: echo $str $path" << endl;
            else {
			    string str = cmdSplit[1];
                target = cmdSplit[2];
			    errorAlert(cmd,target,echo(str,target,curInode,disk));
            }
		}
		else if(cmd=="cat"){
			if (cmdSplit.size() < 2)
                cout << "Usage: cat $path" << endl;
            else {
                target = cmdSplit[1];
			    errorAlert(cmd,target,cat(target,curInode,disk));
            }
		}
		else if(cmd=="rm"){
			if (cmdSplit.size() < 2)
                cout << "Usage: rm $path" << endl;
            else {
                target = cmdSplit[1];
			    errorAlert(cmd,target,rm(target,curInode,disk));
            }
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