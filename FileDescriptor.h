#ifndef DISK_SIMULATOR_FILEDESCRIPTOR_H
#define DISK_SIMULATOR_FILEDESCRIPTOR_H

#include <string>
#include "fsInode.h"

/**
 * FileDescriptor class represents a file descriptor in a filesystem.
 * It holds information about the file, its associated inode, and its usage status.
 */
class FileDescriptor {

    pair<string, fsInode*> file; // Pair containing file name and associated inode
    bool b_inUse; // Indicates whether the file descriptor is currently in use

public:

    /**
     * Constructor to initialize a FileDescriptor object.
     *
     * @param FileName: The name of the file.
     * @param fsi: Pointer to the associated fsInode object.
     */
    FileDescriptor(std::string FileName, fsInode* fsi);

    /**
     * Get the name of the file associated with this file descriptor.
     *
     * @return The name of the file.
     */
    std::string getFileName() const;

    /**
     * Get the associated fsInode pointer for this file descriptor.
     *
     * @return Pointer to the associated fsInode object.
     */
    fsInode* getInode() const;


    /**
     * Get the size of the file associated with this file descriptor.
     *
     * @return The size of the file in bytes, or -1 if the associated inode is nullptr.
     */
    int GetFileSize() const;

    /**
     * Check if the file descriptor is currently in use.
     *
     * @return True if the file descriptor is in use, false otherwise.
     */
    bool isInUse() const;

    /**
     * Set the usage status of the file descriptor.
     *
     * @param _inUse: True to mark the file descriptor as in use, false to mark it as not in use.
     */
    void setInUse(bool _inUse);

    /**
     * Set the name of the file associated with this file descriptor.
     *
     * @param name: The new name for the file.
     */
    void setName(std::string name);
};

#endif //DISK_SIMULATOR_FILEDESCRIPTOR_H
