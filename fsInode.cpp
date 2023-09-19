#include "fsInode.h"

fsInode::fsInode(int _block_size) {
    fileSize = 0;
    blocksInSingleInDirect = 0;
    block_in_use = 0;
    block_size = _block_size;
    directBlock1 = -1;
    directBlock2 = -1;
    directBlock3 = -1;
    singleInDirect = -1;
    doubleInDirect = -1;
    singleBlocksCount = 0;
    blocksInEachSingle = new int[_block_size];
    singleBlocksLocation = new int[_block_size];

    for (int i = 0 ; i < _block_size ; i++)
    {
        blocksInEachSingle[i] = 0;
        singleBlocksLocation[i] = -1;
    }
}

fsInode::fsInode(const fsInode& other) {
    fileSize = other.fileSize;
    block_in_use = other.block_in_use;
    block_size = other.block_size;
    directBlock1 = other.directBlock1;
    directBlock2 = other.directBlock2;
    directBlock3 = other.directBlock3;
    singleInDirect = other.singleInDirect;
    blocksInSingleInDirect = other.blocksInSingleInDirect;
    doubleInDirect = other.doubleInDirect;
    singleBlocksCount = other.singleBlocksCount;


    blocksInEachSingle = new int[block_size];
    singleBlocksLocation = new int[block_size];
    for (int i = 0; i < block_size; i++)
    {
        blocksInEachSingle[i] = other.blocksInEachSingle[i];
        singleBlocksLocation[i] = other.singleBlocksLocation[i];
    }
}

bool fsInode::isSpace()
{
    return (AMOUNT_OF_DIRECT * block_size) + (block_size * block_size) + (block_size * block_size * block_size) <= fileSize;
}

fsInode::~fsInode() {
    delete[] blocksInEachSingle;
    delete[] singleBlocksLocation;
}

void fsInode::setSingleBlockLocation(int index, int location) {
    if (index < 0 || index > 3)
        return;

    singleBlocksLocation[index] = location;
}

int fsInode::getSingleBlockLocation(int index) {
    if (index < 0 || index > 3)
        return -1;

    return static_cast<int>(singleBlocksLocation[index]);
}

int fsInode::getFileSize() const {
    return fileSize;
}

int fsInode::getBlocksInEachSingle(int index) {
    if (index < 0 || index > 3)
        return -1;

    return blocksInEachSingle[index];
}

void fsInode::addBlocksInSingleInDirect(int amount) {
    blocksInSingleInDirect += amount;
}

void fsInode::addSingleBlocksCount(int amount) {
    singleBlocksCount += amount;
}

void fsInode::addBlocksInEachSingle(int index, int amount) {
    blocksInEachSingle[index] += amount;
}

void fsInode::addFileSize(int size) {
    fileSize += size;
}

int fsInode::getBlockInUse() const {
    return block_in_use;
}

void fsInode::addBlockInUse(int num) {
    block_in_use += num;
}

int fsInode::getDirectBlock(int index) const {
    if (index == 1)
        return directBlock1;

    if (index == 2)
        return directBlock2;

    if (index == 3)
        return directBlock3;

    return -1;
}

int fsInode::getSingleInDirect() const {
    return singleInDirect;
}

int fsInode::getBlocksInSingleInDirect() const {
    return blocksInSingleInDirect;
}

int fsInode::getSingleBlocksCount() const {
    return singleBlocksCount;
}

int fsInode::getDoubleInDirect() const {
    return doubleInDirect;
}

int fsInode::getBlockSize() const {
    return block_size;
}

void fsInode::setSingleInDirect(int index) {
    singleInDirect = index;
}

void fsInode::setDoubleInDirect(int num) {
    fsInode::doubleInDirect = num;
}

int fsInode::getInternalFragAmount(int me) const {
    if (me == 1 && singleInDirect != -1) // If I'm direct
        return 0;

    if (me == 2 && doubleInDirect != -1) // If I'm singleInDirect
        return 0;

    if (fileSize % block_size != 0)
        return block_in_use * block_size - fileSize;

    return 0;
}

int fsInode::getInternalFragLocation(int fragAmount, int singleBlock) const {
    if (singleInDirect == -1) // Frag in direct blocks
    {
        if (block_in_use == 3) // Internal fragmentation in block 3
            return (directBlock3 * block_size) + (block_size - fragAmount);

        if (block_in_use == 2) // Internal fragmentation in block 2
            return (directBlock2 * block_size) + (block_size - fragAmount);

        if (block_in_use == 1) // Internal fragmentation in block 1
            return (directBlock1 * block_size) + (block_size - fragAmount);
    }

    return singleBlock + (block_size - fragAmount);
}

int fsInode::getAvailableDirect() const {
    if (directBlock1 == -1)
        return 1;

    if (directBlock2 == -1)
        return 2;

    if (directBlock3 == -1)
        return 3;

    return -1;
}

void fsInode::updateDirectBlock(int block, int location) {
    if (block == 1)
        directBlock1 = location;

    if (block == 2)
        directBlock2 = location;

    if (block == 3)
        directBlock3 = location;
}
