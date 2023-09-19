#ifndef DISK_SIMULATOR_FSDISK_H
#define DISK_SIMULATOR_FSDISK_H


#include <iostream>
#include <map>
#include <vector>
#include <cassert>
#include <cmath>
#include <string.h>
#include "FileDescriptor.h"
#include "fsInode.h"

using namespace std;

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"
#define DISK_SIZE 512
#define MIN_BLOCK_SIZE 2
#define AMOUNT_OF_DIRECT 3

/**
 * fsDisk class represents the disk management system for a filesystem.
 * It manages the disk structure, block allocation, directories, and file descriptors.
 */
class fsDisk {
private:
    FILE* sim_disk_fd; // File for the simulated disk file

    bool b_is_formated; // Indicates whether the disk is formatted
    bool b_is_first_format; // Indicates whether it's the first format
    int blockSize; // Size of each block in bytes
    int currentDiskSize; // Current size of the disk in blocks
    int blocksUsed; // Number of blocks currently in use

    int BitVectorSize; // Size of the BitVector array
    int* BitVector; // Array indicating block occupancy

    map<string, fsInode*> MainDir; // Main directory mapping file names to inodes

    vector<FileDescriptor> openFileDescriptors; // List of open file descriptors
    vector<fsInode*> deletedFiles; // List of deleted fsInodes

    // Private member functions

    /**
     * Get the index of the first free block on the disk.
     *
     * @return The index of the first free block, or -1 if no free blocks are available.
     */
    int getFreeDiskSpace();

    /**
     * Get the index of the first free location in the list of open file descriptors.
     *
     * @return The index of the first free location, or the size of the list if all locations are in use.
     */
    int getFreeLocation();

    /**
     * Get the index of the file descriptor associated with a given file name.
     *
     * @param name: The name of the file to find the descriptor for.
     * @return The index of the file descriptor, or -1 if not found.
     */
    int getFileDescriptor(string name);

    /**
  * Insert a FileDescriptor into the vector of open file descriptors.
  *
  * @param fd: The FileDescriptor to be inserted.
  * @return The index where the FileDescriptor is inserted.
  */
    int insertIntoVector(FileDescriptor fd);

    /**
     * Check if a file with a given name exists in the MainDir map.
     *
     * @param name: The name of the file to check.
     * @return True if the file exists in the map, false otherwise.
     */
    bool isInMap(string name);

    /**
     * Check if a given file descriptor is valid and corresponds to an open file.
     *
     * @param fd: The file descriptor to check.
     * @return True if the file descriptor is valid and in use, false otherwise.
     */
    bool isLegalFD(int fd);

    /**
     * Write a character to the specified location on the simulated disk.
     *
     * @param charToWrite: The character to be written.
     * @param location: The location on the disk to write to.
     * @return 1 if the write is successful, -1 if there's an error.
     */
    int writeLocation(char charToWrite, int location);

    /**
    * Write a block of data to the specified location on the simulated disk.
    *
    * @param writtenAmount: Pointer to store the amount of data written.
    * @param buf: Pointer to the buffer containing data to write.
    * @param amount: The amount of data to write.
    * @param location: The location on the disk to write to, or -1 to allocate a new block.
    * @return The index of the block where data is written, or -1 if there's an error.
    */
    int writeBlock(int* writtenAmount, char*& buf, int amount, int location);


    /**
     * Write data directly to the disk blocks associated with the given inode.
     *
     * @param buf: Pointer to the buffer containing data to write.
     * @param inode: Pointer to the inode associated with the file.
     * @return 0 if no direct blocks are available, 1 if no space left, 2 if write finished, -1 if an error occurred.
     */
    int writeDirect(char*& buf, fsInode* inode);

    /**
    * Write data directly to the single indirect block associated with the given inode.
    *
    * @param buf: Pointer to the buffer containing data to write.
    * @param inode: Pointer to the inode associated with the file.
    * @param indexSingleBlock: The index of the single indirect block.
    * @return The index of the new block written, -1 if an error occurred.
    */
    int writeSingle(char*& buf, fsInode* inode, int indexSingleBlock);

    /**
     * Get the location of the last block in the single indirect block associated with the given inode.
     *
     * @param inode: Pointer to the inode associated with the file.
     * @return The location of the last block in the single indirect block.
     */
    int getLastBlockInSingle(fsInode* inode);

    /**
     * Get the location of the last block in a single indirect block at the specified location.
     *
     * @param location: The location of the single indirect block.
     * @param blocksAmount: The number of blocks in the single indirect block.
     * @return The location of the last block in the single indirect block.
     */
    int getLastBlockInSingle(int location, int blocksAmount);

    /**
     * Write data to the single indirect block associated with the given inode.
     *
     * @param buf: Pointer to the buffer containing data to write.
     * @param inode: Pointer to the inode associated with the file.
     * @return 0 if no space in single indirect block, 1 if no space left on disk, 2 if write finished, -1 if an error occurred.
     */
    int writeSingleInDirect(char*& buf, fsInode* inode);

    /**
    * Write data to the double indirect block associated with the given inode.
    *
    * @param buf: Pointer to the buffer containing data to write.
    * @param inode: Pointer to the inode associated with the file.
    * @return 0 if no space in double indirect block, 1 if no space left on disk,
    *         2 if write finished, -1 if an error occurred.
    */
    int writeDoubleInDirect(char*& buf, fsInode* inode);

    /**
     * Delete a file from the MainDir map and optionally reduce disk size.
     *
     * @param name: The name of the file to delete.
     * @param reduceDiskSize: Flag indicating whether to reduce disk size.
     * @return true if successful, false if the file is not found.
     */
    bool deleteFromMainDir(const std::string& name, bool reduceDiskSize);

    /**
    * Read data from the disk into the buffer.
    *
    * @param len: The total amount of data to read.
    * @param buf: Pointer to the buffer to store the read data.
    * @param buf_index: The current index in the buffer.
    * @return The amount of data read.
    */
    int makeRead(int len, char*& buf, int buf_index);

    /**
     * Read data from single indirect blocks associated with an inode.
     *
     * @param len: Pointer to the remaining length of data to be read.
     * @param buf: Pointer to the buffer to store the read data.
     * @param buf_index: Pointer to the current index in the buffer.
     * @param singleAddress: The address of the single indirect block.
     * @param blocksAmount: The number of blocks to read from the single indirect block.
     * @param isIndex: Flag indicating whether 'singleAddress' is an index or an absolute address.
     * @return 1 if successful, -1 if an error occurred.
     */
    int readSingleInDirect(int *len, char*& buf, int *buf_index, int singleAddress, int blocksAmount, bool isIndex);

    /**
     * Delete single indirect blocks and their associated data.
     *
     * @param singleLocation: The location of the single indirect block.
     * @param blocksAmount: The number of blocks in the single indirect block.
     * @return true if successful, false otherwise.
     */
    bool deleteSingleBlock(int singleLocation, int blocksAmount);

    /**
      * Delete blocks associated with an inode.
      *
      * @param inode: Pointer to the inode from which to delete blocks.
      * @return 1 if successful, indicating that the blocks were deleted.
      */
    int deleteBlocks(fsInode* inode);

    /**
     * Count the total number of blocks used by an inode, including indirect blocks.
     *
     * @param inode: Pointer to the inode to count blocks for.
     * @return The total number of blocks used by the inode.
     */
    static int countUsedBlocks(fsInode* inode);

    /**
     * Check if there is enough space on the disk to copy a certain number of blocks.
     *
     * @param requiredBlocks: The number of blocks required for copying.
     * @param removeBlocks: The number of blocks to remove.
     * @return true if there is enough space, false otherwise.
     */
    bool isEnoughSpaceToCopy(int requiredBlocks, int removeBlocks) const;

    /**
     * Initialize the disk's state.
     */
    void init();

    /**
     * Delete all entries from the MainDir map and free associated resources.
     */
    void deleteMap();

    /**
     * Print an error message and return an error code.
     *
     * @param text: The error message to be printed.
     * @return -1 to indicate an error.
     */
    static int makeError(string text);


    static char decToBinaryChar(int n);


    bool isStringOnlySpaces(const string &str)

public:

    /**
  * Constructor for the fsDisk class.
  * Initializes the simulated disk and sets initial properties.
  */
    fsDisk();

    /**
     * List all open file descriptors and display disk content.
     */
    void listAll();

    /**
  * Constructor for the fsDisk class.
  * Initializes the simulated disk and sets initial properties.
  */
    void fsFormat(int blockSize = 4);

    /**
  * Constructor for the fsDisk class.
  * Initializes the simulated disk and sets initial properties.
  */
    int CreateFile(std::string fileName);

    /**
  * Constructor for the fsDisk class.
  * Initializes the simulated disk and sets initial properties.
  */
    int OpenFile(std::string FileName);

    /**
     * Close a previously opened file.
     *
     * @param fd: The index of the file descriptor to be closed.
     * @return The name of the closed file, or an error code.
     */
    string CloseFile(int fd);

    /**
     * Write data to a file.
     *
     * @param fd: The index of the file descriptor to write to.
     * @param buf: The buffer containing data to be written.
     * @param len: The length of data to write.
     * @return 1 to indicate success or an error code.
     */
    int WriteToFile(int fd, char *buf, int len);

    /**
   * Read data from a file.
   *
   * @param fd: The index of the file descriptor to read from.
   * @param buf: The buffer to store the read data.
   * @param len: The maximum length of data to read.
   * @return 1 to indicate success or an error code.
   */
    int ReadFromFile(int fd, char *buf, int len);

    /**
     * Delete a file from the file system.
     *
     * @param FileName: The name of the file to be deleted.
     * @return 1 to indicate success or an error code.
     */
    int DelFile(std::string FileName);

    /**
  * Copy a file to a new destination.
  *
  * @param srcFileName: The name of the source file to copy.
  * @param destFileName: The name of the destination file.
  * @return 1 to indicate success or an error code.
  */
    int CopyFile(std::string srcFileName, std::string destFileName);

    /**
     * Rename a file.
     *
     * @param oldFileName: The current name of the file.
     * @param newFileName: The new name for the file.
     * @return 1 to indicate success or an error code.
     */
    int RenameFile(std::string oldFileName, std::string newFileName);

    /**
     * Destructor for the fsDisk class.
     */
    ~fsDisk();
};



#endif //DISK_SIMULATOR_FSDISK_H
