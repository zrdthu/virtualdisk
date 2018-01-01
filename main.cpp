#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

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

    string cmd, target;
    while(cmd != "exit" && cmd != "logout") {
        cout << ">> ";
        cin >> cmd;
        if (cmd == "pwd")
            pwd(curInode, disk);
        else if (cmd == "cd") {
            cin >> target;
            errorAlert(cmd, target, cd(target, curInode, disk));
        }
        else if (cmd == "mkdir") {
            cin >> target;
            errorAlert(cmd, target, mkdir(target, curInode, disk));
        }
        else if (cmd == "ls") {
            cin >> target;
            errorAlert(cmd, target, ls(target, curInode, disk));
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