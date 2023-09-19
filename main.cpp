#include <iostream>
#include "fsDisk.h"

using namespace std;


int main() {
    int blockSize;
    string fileName;
    string fileName2;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;

    fsDisk *fs = new fsDisk();
    int cmd_;
    while(true) {
        cin >> cmd_;

        switch (cmd_)
        {
            case 0:   // exit
                delete fs;
                exit(0);

            case 1:  // list-file
                fs->listAll();
                break;

            case 2:    // format
                cin >> blockSize;
                fs->fsFormat(blockSize);
                cout << "Formatted disk with block size of " << blockSize << endl;
                break;

            case 3:    // create-file
                cin >> fileName;
                _fd = fs->CreateFile(fileName);
                if (_fd != -1)
                    cout << "--Created File--\n" << "File Name: " << fileName << "\nFile Descriptor #: " << _fd << endl;
                break;

            case 4:  // open-file
                cin >> fileName;
                _fd = fs->OpenFile(fileName);
                if (_fd != -1)
                    cout << "--Opened File--\n" << "File Name: " << fileName << "\nFile Descriptor #: " << _fd << endl;
                break;

            case 5:  // close-file
                cin >> _fd;
                fileName = fs->CloseFile(_fd);
                if (fileName != "-1")
                   cout << "--Closed File--\n" << "File Name: " << fileName << "\nFile Descriptor #: " << _fd << endl;
                break;

            case 6:   // write-file
                cin >> _fd;
                cin >> str_to_write;
                if (fs->WriteToFile(_fd , str_to_write , strlen(str_to_write)) == 1)
                    cout << "Wrote To File Successfully" << endl;
                break;

            case 7:    // read-file
                cin >> _fd;
                cin >> size_to_read ;
                if (fs->ReadFromFile( _fd , str_to_read , size_to_read) == 1)
                   cout << "Read From File: " << str_to_read << endl;
                break;

            case 8:   // delete file
                cin >> fileName;
                _fd = fs->DelFile(fileName);
                if (_fd != -1)
                    cout << "Deleted File Successfully" << endl;
                break;

            case 9:   // copy file
                cin >> fileName;
                cin >> fileName2;
                if (fs->CopyFile(fileName, fileName2) != -1)
                    cout << "--Copied File--\n" << "Source File Name: " << fileName << "\nNew File Name: " << fileName2 << endl;
                break;

            case 10:  // rename file
                cin >> fileName;
                cin >> fileName2;
                if (fs->RenameFile(fileName, fileName2) != -1)
                    cout << "--Renamed File--\n" << "Previous File Name: " << fileName << "\nNew File Name: " << fileName2 << endl;
                break;

            default:
                break;
        }
    }
}