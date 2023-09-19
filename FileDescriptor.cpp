#include "FileDescriptor.h"

FileDescriptor::FileDescriptor(std::string FileName, fsInode* fsi) {
    file.first = FileName;
    file.second = fsi;
    b_inUse = true;
}

std::string FileDescriptor::getFileName() const {
    return file.first;
}

fsInode* FileDescriptor::getInode() const {
    return file.second;
}

int FileDescriptor::GetFileSize() const {
    if (file.second == nullptr)
        return -1;

    return file.second->getFileSize();
}

bool FileDescriptor::isInUse() const {
    return (b_inUse);
}

void FileDescriptor::setInUse(bool _inUse) {
    b_inUse = _inUse ;
}

void FileDescriptor::setName(std::string name) {
    file.first = name;
}
