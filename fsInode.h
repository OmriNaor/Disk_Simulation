#ifndef DISK_SIMULATOR_FSINODE_H
#define DISK_SIMULATOR_FSINODE_H

#define AMOUNT_OF_DIRECT 3

class fsInode {
    int fileSize;                   // Size of the file in bytes
    int block_in_use;               // Total number of blocks in use

    // Direct blocks
    int directBlock1;               // Location of the 1st direct block
    int directBlock2;               // Location of the 2nd direct block
    int directBlock3;               // Location of the 3rd direct block

    // Single indirect block
    int singleInDirect;             // Location of the single indirect block
    int blocksInSingleInDirect;     // Number of blocks allocated in the single indirect block

    // Double indirect block
    int doubleInDirect;             // Location of the double indirect block
    int singleBlocksCount;          // Total number of single blocks
    int* blocksInEachSingle;        // Array to store the number of blocks allocated in each single block
    int* singleBlocksLocation;      // Array to store the location of each single block

    int block_size;                 // Block size of the filesystem

public:

    /**
     * Constructor to initialize an fsInode object.
     *
     * @param _block_size: The block size of the filesystem.
    */
    explicit fsInode(int _block_size);

    /**
    * Copy constructor to create a deep copy of an fsInode object.
    *
    * @param other: The fsInode object to be copied.
    */
    fsInode(const fsInode& other);

    /**
    * Destructor to clean up dynamically allocated resources.
    */
    ~fsInode();

    /**
  * Set the location of a single block in the singleBlocksLocation array.
  *
  * @param index: The index of the single block location to be set.
  * @param location: The location to set for the single block.
  */
    void setSingleBlockLocation(int index, int location);


    /**
     * Get the location of a single block at the specified index from the singleBlocksLocation array.
     *
     * @param index: The index of the single block location to retrieve.
     * @return The location of the single block at the specified index, or -1 if the index is out of range.
     */
    int getSingleBlockLocation(int index);

    bool isSpace();


    /**
     * Get the file size associated with this inode.
     *
     * @return The file size in bytes.
     */
    int getFileSize() const;

     /**
      * Get the number of blocks allocated in a specific single block from the blocksInEachSingle array.
      *
      * @param index: The index of the single block to retrieve the allocated block count for.
      * @return The number of blocks allocated in the specified single block, or -1 if the index is out of range.
      */
    int getBlocksInEachSingle(int index);


    void addBlocksInSingleInDirect(int amount);

    void addSingleBlocksCount(int amount);

    void addBlocksInEachSingle(int index, int amount);

    /**
    * Add to the file size associated with this inode.
    *
    * @param size: The size to be added to the file size.
    */
    void addFileSize(int size);

    /**
      * Get the total number of blocks in use by this inode.
      *
      * @return The number of blocks in use.
      */
    int getBlockInUse() const;

    /**
    * Add a number of blocks to the count of blocks in use.
    *
    * @param num: The number of blocks to add.
    */
    void addBlockInUse(int num);

    /**
  * Get the location of a direct block at the specified index.
  *
  * @param index: The index of the direct block to retrieve (1, 2, or 3).
  * @return The location of the direct block at the specified index, or -1 if the index is invalid.
  */
    int getDirectBlock(int index) const;

    int getSingleInDirect() const;

    int getBlocksInSingleInDirect() const;

    int getSingleBlocksCount() const;

    int getDoubleInDirect() const;

    int getBlockSize() const;

    void setSingleInDirect(int index);

    void setDoubleInDirect(int num);

    /**
     * Get the internal fragmentation amount for a specific block.
     *
     * @param me: The block index to calculate internal fragmentation for.
     * @return The internal fragmentation amount in bytes.
     */
    int getInternalFragAmount(int me) const;

    /**
  * Get the location for internal fragmentation in a specific block.
  *
  * @param fragAmount: The amount of internal fragmentation.
  * @param singleBlock: The location of the single block (default: 0).
  * @return The location for internal fragmentation.
  */
    int getInternalFragLocation(int fragAmount, int singleBlock = 0) const;

    /**
     * Get the index of an available direct block.
     *
     * @return The index of an available direct block (1, 2, or 3), or -1 if none are available.
     */
    int getAvailableDirect() const;

    /**
     * Update the location of a direct block.
     *
     * @param block: The index of the direct block to update (1, 2, or 3).
     * @param location: The new location to set for the direct block.
     */
    void updateDirectBlock(int block, int location);
};

#endif //DISK_SIMULATOR_FSINODE_H
