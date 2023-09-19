#include "fsDisk.h"

int fsDisk::getFreeDiskSpace()
{
    if (BitVector == nullptr)
        return - 1;

    for (int i = 0 ; i < BitVectorSize ; i++)
        if (BitVector[i] == 0)
            return i;

    return -1;
}

int fsDisk::getFreeLocation()
{
    for (int i = 0 ; i < openFileDescriptors.size() ; i++)
        if (!openFileDescriptors[i].isInUse())
            return i;

    return openFileDescriptors.size();
}

int fsDisk::getFileDescriptor(string name)
{

    for (int i = 0 ; i < openFileDescriptors.size() ; i++)
        if (openFileDescriptors[i].getFileName() == name)
            return i;

    return -1;
}

int fsDisk::insertIntoVector(FileDescriptor fd)
{
    int index = getFreeLocation();

    if (index < openFileDescriptors.size())
        openFileDescriptors[index] = fd;

    else
        openFileDescriptors.push_back(fd);

    return index;
}

bool fsDisk::isInMap(string name)
{
    return !(MainDir.find(name) == MainDir.end());
}

bool fsDisk::isLegalFD(int fd)
{
    return (fd >= 0 && fd < openFileDescriptors.size() && openFileDescriptors[fd].isInUse());
}


int fsDisk::writeLocation(char charToWrite, int location)
{
    // Reposition the file offset to the specified location.
    if (fseek(sim_disk_fd, location, SEEK_SET) != 0)
        return -1; // Return -1 if there's an error.

    // Write the data to the file at the specified location.
    if (fwrite(&charToWrite, 1, 1, sim_disk_fd) == -1)
        return -1;

    return 1;
}

int fsDisk::writeBlock(int* writtenAmount, char*& buf, int amount, int location)
{
    int index = location;
    size_t file_offset = location;

    if (location == -1)
    {
        index = getFreeDiskSpace();
        if (index == -1)
            return -1;

        // Calculate the absolute file offset to write to
        file_offset = index * blockSize;
    }

    // Reposition the file offset to the specified location.
    if (fseek(sim_disk_fd, file_offset, SEEK_SET) != 0)
        return -1; // Return -1 if there's an error.

    size_t bytes_to_write = (strlen(buf) <= amount) ? strlen(buf) : amount;

    // Write the data to the file at the specified location.
    size_t bytes_written = fwrite(buf, 1, bytes_to_write, sim_disk_fd);

    // Check if the write operation was successful.
    if (bytes_written != bytes_to_write)
        return -1; // Return -1 if there was an error.

    currentDiskSize += bytes_written;
    *writtenAmount = bytes_written;
    buf += bytes_written;

    if (location == -1)
    {
        BitVector[index] = 1;
        blocksUsed++;
    }

    fflush(sim_disk_fd);
    return index;
}

int fsDisk::writeDirect(char*& buf, fsInode* inode)
{

    int written;
    int fragAmount = inode->getInternalFragAmount(1);
    if (fragAmount != 0)
    {
        int fragLocation = inode->getInternalFragLocation(fragAmount);
        writeBlock(&written, buf, fragAmount, fragLocation);
        inode->addFileSize(written);
    }

    int directBlock = inode->getAvailableDirect();
    if (directBlock == -1) // No direct blocks available
        return 0;

    if (strlen(buf) <= 0) // Nothing to write
        return 1;

    if (currentDiskSize + blockSize > DISK_SIZE) // No space on the DISK_SIZE
        return 1;



    directBlock = inode->getAvailableDirect();
    if (directBlock == -1) // No direct blocks available
        return 0;

    int index = writeBlock(&written, buf, blockSize, -1);

    if (index == -1) // Failed
        return -1;

    inode->updateDirectBlock(directBlock, index);
    inode->addFileSize(written);
    inode->addBlockInUse(1);
    return 2; // Finished
}

int fsDisk::writeSingle(char*& buf, fsInode* inode, int indexSingleBlock)
{
    int written;

    if (strlen(buf) <= 0) // Nothing to write
        return 1;

    int singleIndex = getFreeDiskSpace();
    if (singleIndex == -1)
        return -1;

    BitVector[singleIndex] = 1;
    blocksUsed++;


    int index = writeBlock(&written, buf, blockSize, -1);

    if (index == -1)
        return -1;

    inode->addFileSize(written);

    // Write the new block under the singleInDirect
    if (writeLocation(decToBinaryChar(index), singleIndex * blockSize) == -1)
        return -1;

    return singleIndex;
}

int fsDisk::getLastBlockInSingle(fsInode* inode)
{
    char* buf = new char[blockSize];

    fseek(sim_disk_fd, inode->getSingleInDirect() * blockSize, SEEK_SET);
    makeRead(inode->getBlocksInSingleInDirect(), buf, 0);
    int location = static_cast<int>(buf[inode->getBlocksInSingleInDirect() - 1]) * blockSize;

    delete[] buf;
    return location;
}

int fsDisk::getLastBlockInSingle(int location, int blocksAmount)
{
    char* buf = new char[blockSize];

    fseek(sim_disk_fd, location, SEEK_SET);
    makeRead(blocksAmount, buf, 0);
    location = static_cast<int>(buf[blocksAmount - 1]) * blockSize;

    delete[] buf;
    return location;
}

int fsDisk::writeSingleInDirect(char*& buf, fsInode* inode)
{

    int written;
    int fragAmount = inode->getInternalFragAmount(2);
    if (fragAmount != 0 && strlen(buf) > 0)
    {
        int location = getLastBlockInSingle(inode);
        int fragLocation = inode->getInternalFragLocation(fragAmount, location);
        writeBlock(&written, buf, fragAmount, fragLocation);
        inode->addFileSize(written);
    }

    if (inode->getBlocksInSingleInDirect() >= blockSize)
        return 0; // Not finished but also no space in single

    if (strlen(buf) <= 0) // Nothing to write
        return 1;

    if (inode->getSingleInDirect() == -1 && blocksUsed + 1 >= DISK_SIZE / blockSize) // No space for data
        return 0;

    int isFirst = (inode-> getSingleInDirect() == -1) ? blockSize : 0;
    if (currentDiskSize + isFirst + blockSize > DISK_SIZE) // No space on the DISK_SIZE
        return 1;

    int index;

    if (inode->getSingleInDirect() == -1)
    {
        index = writeSingle(buf, inode, -1);

        if (index == -1)
            return -1;

        inode->setSingleInDirect(index);
    }

    else
    {
        index = writeBlock(&written, buf, blockSize, -1);

        if (index == -1)
            return -1;

        inode->addFileSize(written);


        int singleLocation = inode->getSingleInDirect() * blockSize + inode->getBlocksInSingleInDirect();
        index = writeLocation(decToBinaryChar(index), singleLocation); // Write the location of the fresh written block

        if (index == -1)
            return -1;

        //     inode->addFileSize(written); // Keep?

    }

    inode->addBlocksInSingleInDirect(1);
    inode->addBlockInUse(1);
    return 2; // Finished but unknown need more
}

int fsDisk::writeDoubleInDirect(char*& buf, fsInode* inode)
{
    int fragAmount = inode->getInternalFragAmount(3);
    int index;
    int written;

    if (fragAmount != 0 && strlen(buf) > 0)
    {
        int lastSingle = inode->getSingleBlockLocation(inode->getSingleBlocksCount() - 1);
        int location = getLastBlockInSingle(lastSingle, inode->getBlocksInEachSingle(inode->getSingleBlocksCount() - 1));
        int fragLocation = inode->getInternalFragLocation(fragAmount, location);

        writeBlock(&written, buf, fragAmount, fragLocation);
        inode->addFileSize(written);
    }

    if (strlen(buf) <= 0) // Nothing to write
        return 1;

    if (inode->isSpace()) // inode is full
        return 1;


    if (inode->getSingleBlocksCount() > blockSize)
        return 0; // Not finished but also no space in doubleInDirect


    if (inode->getDoubleInDirect() == -1 &&  2*blockSize + currentDiskSize > DISK_SIZE)
        return 0; // No space on the disk to create new doubleInDirect


    // First doubleInDirect
    if (inode->getDoubleInDirect() == -1)
    {
        index = getFreeDiskSpace();
        if (index == -1)
            return -1;

        if (blocksUsed + 2 >= DISK_SIZE / blockSize) // No space for data
            return -1;

        BitVector[index] = 1;
        blocksUsed++;
        inode->setDoubleInDirect(index);

        int singleIndex = writeSingle(buf, inode, -1);
        if (singleIndex == -1)
            return -1;

        // Write the new single block under the doubleInDirect
        writeLocation(decToBinaryChar(singleIndex), index * blockSize);

        inode->addSingleBlocksCount(1); // Double has one more singleInDirect
        inode->addBlocksInEachSingle(0, 1); // singleInDirect of doubleInDirect has one more block
        inode->setSingleBlockLocation(0, singleIndex * blockSize); // First singleInDirect location of the doubleInDirect
        inode->addBlockInUse(1);

        return 2; // Finished
    }


    if (blockSize + currentDiskSize > DISK_SIZE)
        return 0; // No space on the disk to create new block

    int blocksAmountInLastSingle = inode->getBlocksInEachSingle(inode->getSingleBlocksCount() - 1);
    if (blocksAmountInLastSingle < blockSize) // If the last single is not full
    {
        index = writeBlock(&written, buf, blockSize, -1);

        if (index == -1)
            return -1;

        inode->addFileSize(written);


        int singleLocation = inode->getSingleBlockLocation(inode->getSingleBlocksCount() - 1) + blocksAmountInLastSingle;
        index = writeLocation(decToBinaryChar(index), singleLocation);

        if (index == -1)
            return -1;

        inode->addBlockInUse(1);
        inode->addBlocksInEachSingle(inode->getSingleBlocksCount() - 1, 1); // singleInDirect of doubleInDirect has one more block

        return 2;
    }


    if (2*blockSize + currentDiskSize > DISK_SIZE)
        return 0; // No space on the disk to create new single

    // Continue to create a new single
    if (blocksUsed + 1 >= DISK_SIZE / blockSize) // No space for data
        return -1;

    int singleIndex = writeSingle(buf, inode, -1);
    if (singleIndex == -1)
        return -1;

    // Write the new single block under the doubleInDirect
    int writeIndex = inode->getDoubleInDirect() * blockSize + inode->getSingleBlocksCount();
    writeLocation(decToBinaryChar(singleIndex), writeIndex);

    inode->addSingleBlocksCount(1); // Double has one more singleInDirect
    inode->addBlocksInEachSingle(inode->getSingleBlocksCount() - 1, 1); // singleInDirect of doubleInDirect has one more block
    inode->setSingleBlockLocation(inode->getSingleBlocksCount() - 1, singleIndex * blockSize); // First singleInDirect location of the doubleInDirect
    inode->addBlockInUse(1);

    return 2; // Finished
}


bool fsDisk::isStringOnlySpaces(const string &str)
{
    for (char c : str)
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
            // If any non-space character is found, return false.
            return false;

    // If the loop completes without finding any non-space character, return true.
    return true;
}

bool fsDisk::deleteFromMainDir(const string& name, bool reduceDiskSize)
{
    if (!isInMap(name))
        return false; // File not found

    auto it = MainDir.find(name);

    if (reduceDiskSize)
    {
        currentDiskSize -= it->second->getFileSize();
        it->second->addFileSize(-it->second->getFileSize());
    }

    deletedFiles.push_back(it->second);
    MainDir.erase(it); // Erase the key-value pair from the map.
    return true; // Return true to indicate success.
}

int fsDisk::makeRead(int len, char*& buf, int buf_index)
{
    int amountToRead;
    char* text = new char[blockSize];

    len > blockSize ? amountToRead = blockSize : amountToRead = len;

    fread(text, 1, amountToRead, sim_disk_fd);

    // Copy the read data from 'text' to 'buf' at the correct position
    memcpy(buf + buf_index, text, amountToRead);

    // Update the buffer index for the next iteration
    buf_index += amountToRead;

    delete[] text;

    // return the read amount
    return amountToRead;
}

int fsDisk::readSingleInDirect(int *len, char*& buf, int *buf_index, int singleAddress, int blocksAmount, bool isIndex)
{

    char* pointers = new char[blockSize];
    int readBytes;

    if(isIndex)
        singleAddress *= blockSize;


    // Read the singleInDirect pointers
    if (fseek(sim_disk_fd, singleAddress, SEEK_SET) != 0)
    {
        delete[] pointers;
        return -1;
    }

    fread(pointers, 1, blockSize, sim_disk_fd);


    for(int i = 0 ; i < blocksAmount ; i++)
    {

        int location = static_cast<int>(pointers[i]);

        if (fseek(sim_disk_fd, location * blockSize, SEEK_SET) != 0)
        {
            delete[] pointers;
            return -1;
        }

        // Update len to the remaining length of data to be read
        readBytes = makeRead(*len, buf, *buf_index);
        *buf_index += readBytes;
        *len -= readBytes;
    }

    delete[] pointers;
    return 1;
}



bool fsDisk::deleteSingleBlock(int singleLocation, int blocksAmount)
{
    int blockLocation;
    int currentSingleBlocksAmount = blocksAmount;

    // Delete the singleInDirect blocks
    for (int i = 0 ; i < blocksAmount ; i++ )
    {
        blockLocation = getLastBlockInSingle(singleLocation, currentSingleBlocksAmount);

        BitVector[blockLocation / blockSize] = 0;
        blocksUsed--;
        currentSingleBlocksAmount--;
    }


    BitVector[singleLocation / blockSize] = 0;
    blocksUsed--;

    return true;
}


int fsDisk::deleteBlocks(fsInode* inode)
{
    int blockLocation;
    int max;
    int amountOfBlocks = inode->getBlockInUse();
    // Delete the direct blocks
    max = AMOUNT_OF_DIRECT < amountOfBlocks ? AMOUNT_OF_DIRECT : amountOfBlocks; // Max loop iterations
    for (int i = 1; i <= max; i++)
    {
        blockLocation = inode->getDirectBlock(i);

        BitVector[blockLocation] = 0; // Mark the block as free
        blocksUsed--;
    }

    amountOfBlocks -= max;


    // Delete single indirect blocks
    if (inode->getSingleInDirect() != -1)
    {
        amountOfBlocks -= inode->getBlocksInSingleInDirect();
        deleteSingleBlock(inode->getSingleInDirect() * blockSize, inode->getBlocksInSingleInDirect());
    }


    // Delete double indirect blocks
    if (inode->getDoubleInDirect() != -1)
    {
        for (int i = 0; i < amountOfBlocks; i++)
            deleteSingleBlock(inode->getSingleBlockLocation(i), inode->getBlocksInEachSingle(i));

        BitVector[inode->getDoubleInDirect()] = 0; // Mark the doubleInDirect as free
        blocksUsed--;
    }


    currentDiskSize -= inode->getFileSize();
    inode->addFileSize(-inode->getFileSize());
    return 1; // Successful block deletion
}

int fsDisk::countUsedBlocks(fsInode* inode)
{
    int requiredBlocks = inode->getBlockInUse();

    if (inode->getSingleInDirect() != -1)
        requiredBlocks++;

    if (inode->getDoubleInDirect() != -1)
    {
        requiredBlocks++; // For the double indirect block
        requiredBlocks += inode->getSingleBlocksCount();
    }

    return requiredBlocks;
}

bool fsDisk::isEnoughSpaceToCopy(int requiredBlocks, int removeBlocks) const
{
    return  (BitVectorSize - blocksUsed - requiredBlocks + removeBlocks >= 0);
}

void fsDisk::init()
{
    for (int i=0; i < DISK_SIZE ; i++)
    {
        int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
        ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd );
        assert(ret_val == 1);
    }
    fflush(sim_disk_fd);
    currentDiskSize = 0;
    blocksUsed = 0;
    BitVectorSize = 0;
    BitVector = nullptr;
}

void fsDisk::deleteMap()
{
    while (!MainDir.empty())
    {
        auto it = MainDir.begin();

        // Delete the dynamically allocated fsInode object
        delete it->second;

        // Erase the map element
        MainDir.erase(it);
    }
}

int fsDisk::makeError(string text)
{
    cout << text << endl;
    return -1;
}


fsDisk::fsDisk() {
    sim_disk_fd = fopen( DISK_SIM_FILE , "w+" );
    assert(sim_disk_fd);
    init();
    b_is_first_format = true;
}


void fsDisk::listAll() {
    int i = 0;
    for (auto it = begin (openFileDescriptors); it != end (openFileDescriptors); ++it)
    {
        cout << "Index: " << i << "\tFile Name: " << it->getFileName() <<  "\tIs Opened: " << it->isInUse() << "\tFile Size: " << it->GetFileSize() << endl;
        i++;
    }
    char bufy;
    cout << "Disk content: '" ;
    for (i=0; i < DISK_SIZE ; i++) {
        int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
        ret_val = fread(  &bufy , 1 , 1, sim_disk_fd );
        cout << bufy;
    }
    cout << "'" << endl;


}

// ------------------------------------------------------------------------
void fsDisk::fsFormat(int blockSize)
{
    if (blockSize < MIN_BLOCK_SIZE || blockSize > DISK_SIZE)
    {
        makeError("ERR");
        return;
    }

    this->blockSize = blockSize;

    if (!b_is_first_format)
    {
        deleteMap();
        delete[] BitVector;
        init();
        MainDir.clear();
        openFileDescriptors.clear();
    }

    b_is_first_format = false;
    b_is_formated = true;
    this->blockSize = blockSize;

    BitVectorSize = DISK_SIZE / this->blockSize;
    BitVector = new int[BitVectorSize];
    assert(BitVector);

    for (int i = 0; i < BitVectorSize; ++i)
        BitVector[i] = 0; // Initialize to 0 to indicate free blocks
}

// ------------------------------------------------------------------------
int fsDisk::CreateFile(string fileName)
{
    if (!b_is_formated || isInMap(fileName))
        return makeError("ERR");

    auto* new_file = new fsInode(blockSize);
    MainDir[fileName] = new_file;

    FileDescriptor new_fd(fileName, new_file);

    return insertIntoVector(new_fd);
}

// ------------------------------------------------------------------------
int fsDisk::OpenFile(string FileName)
{

    // Check if the file exists
    if (!b_is_formated || !isInMap(FileName)) // File was never created or disk wasn't formatted
        return makeError("ERR");

    // Check if the file is in the vector already
    int index = getFileDescriptor(FileName);

    if (index != -1)
    {  // File is already inside the vector
        if (openFileDescriptors[index].isInUse()) // File is already open
            return makeError("ERR");
    }

    // Load the file into the vector
    map<std::string, fsInode*>::iterator it = MainDir.find(FileName);
    FileDescriptor fd(it->first, it->second);
    return insertIntoVector(fd);
}


// ------------------------------------------------------------------------
string fsDisk::CloseFile(int fd)
{
    if (!b_is_formated) // Disk wasn't formatted
    {
        makeError("ERR");
        return "-1";
    }

    if (fd < 0 || fd >= openFileDescriptors.size()) // FD index is illegal
    {
        makeError("ERR");
        return "-1";
    }

    if (!openFileDescriptors[fd].isInUse()) // FD is already closed
    {
        makeError("ERR");
        return "-1";
    }

    openFileDescriptors[fd].setInUse(false);
    return openFileDescriptors[fd].getFileName();
}


// ------------------------------------------------------------------------
int fsDisk::WriteToFile(int fd, char *buf, int len)
{
    if (!b_is_formated || !isLegalFD(fd))
        return makeError("ERR");

    fsInode* inode = openFileDescriptors[fd].getInode();

    if (inode->isSpace()) // No space to write into the specific file
        return makeError("ERR");

    if (inode->getInternalFragAmount(1) == 0 && inode->getInternalFragAmount(2) == 0 && inode->getInternalFragAmount(3) == 0
        && getFreeDiskSpace() == -1)
        return makeError("ERR");

    // Create a dynamically allocated character array for the truncated data
    char* truncatedData = new char[len + 1]; // Add 1 for the null terminator

    size_t originalLength = strlen(buf);

    if (len <= originalLength)
    {
        // Copy the truncated portion of buf into the dynamically allocated variable
        strncpy(truncatedData, buf, len);
        truncatedData[len] = '\0'; // Null-terminate the copied portion
    }
    else
    {
        // Copy the entire original buf into the dynamically allocated variable
        strncpy(truncatedData, buf, originalLength);
        truncatedData[originalLength] = '\0'; // Null-terminate the copied portion
    }

    // Store a separate pointer to the truncated data for writing
    char* writePtr = truncatedData;

    // Write data using different write strategies
    while (writeDirect(writePtr, inode) == 2);
    while (writeSingleInDirect(writePtr, inode) == 2);
    while (writeDoubleInDirect(writePtr, inode) == 2);

    // Clean up: delete the dynamically allocated buffer
    delete[] truncatedData;

    return 1;
}


// ------------------------------------------------------------------------
int fsDisk::ReadFromFile(int fd, char *buf, int len)
{
    buf[0] = '\0';
    if (!b_is_formated || !isLegalFD(fd) || len < 0)
        return makeError("ERR");

    fsInode* inode = openFileDescriptors[fd].getInode();

    if (!openFileDescriptors[fd].isInUse()) // File is closed
        return makeError("ERR");

    int blocksToRead = ceil(static_cast<double>(len) / blockSize);

    if (len > inode->getFileSize())
        len = inode->getFileSize();

    if (len <= 0) // Nothing to read from the file - finish
        return 1;

    int index;
    int file_offset;
    int buf_index = 0;
    int readBytes;

    // Read from direct blocks
    for (int i = 1; i <= 3 && i <= blocksToRead; i++)
    {
        index = inode->getDirectBlock(i);
        file_offset = index * blockSize;

        if (fseek(sim_disk_fd, file_offset, SEEK_SET) != 0)
            return makeError("ERR");

        // Update len to the remaining length of data to be read
        readBytes = makeRead(len, buf, buf_index);
        buf_index += readBytes;
        len -= readBytes;
    }

    // Read from singleInDirect
    blocksToRead -= AMOUNT_OF_DIRECT;

    if (blocksToRead > 0)
    {
        int blocksAmount = inode->getBlocksInSingleInDirect();
        if (blocksAmount > blocksToRead)
            blocksAmount = blocksToRead;

        readSingleInDirect(&len, buf, &buf_index, inode->getSingleInDirect(), blocksAmount, true);
    }

    blocksToRead -= blockSize;

    // Read from doubleInDirect
    if (blocksToRead > 0)
    {
        int blocksAmount = inode->getSingleBlocksCount();
        if (blocksAmount > blocksToRead)
            blocksAmount = blocksToRead;

        for (int i = 0; i < blocksAmount; i++)
        {
            readSingleInDirect(&len, buf, &buf_index, inode->getSingleBlockLocation(i),
                               inode->getBlocksInEachSingle(i), false);
        }
    }

    buf[buf_index] = '\0';

    return 1;
}


// ------------------------------------------------------------------------
int fsDisk::DelFile(string FileName)
{
    if (!b_is_formated || !isInMap(FileName)) // File doesn't exist
        return makeError("ERR");

    int index = getFileDescriptor(FileName);
    if (index > -1 && openFileDescriptors[index].isInUse()) // File is opened
        return makeError("ERR");

    /* The key exists in the map and is closed, delete the fsInode and erase the key-value pair. */
    auto it = MainDir.find(FileName);
    deleteBlocks(it->second);
    deleteFromMainDir(FileName, true);

    if (index > -1)
        openFileDescriptors[index].setName("");

    return 1; // 1 to indicate success. Can return index but what if it's closed and not in the vector (meaning index is -1)
}


// ------------------------------------------------------------------------
int fsDisk::CopyFile(string srcFileName, string destFileName)
{
    if (!b_is_formated)
        return makeError("ERR");

    int fd = getFileDescriptor(srcFileName);
    if (!isInMap(srcFileName) || srcFileName == destFileName || (fd != -1 && openFileDescriptors[fd].isInUse()))
        return makeError("ERR");

    auto it = MainDir.find(srcFileName);

    int requiredBlocks = countUsedBlocks(it->second);
    bool isOverRide = isInMap(destFileName);

    // Check if destFileName already exists
    if (isOverRide)
    {
        auto destIt = MainDir.find(destFileName);

        if (!isEnoughSpaceToCopy(requiredBlocks, countUsedBlocks(destIt->second)))
            return makeError("ERR"); // Not enough space

        int index = getFileDescriptor(destFileName);
        if (index > -1 && openFileDescriptors[index].isInUse()) // File is opened
            return makeError("ERR");

        DelFile(destFileName);
    }
    else if (!isEnoughSpaceToCopy(requiredBlocks, 0))
        return makeError("ERR"); // Not enough space

    // Open the given file
    int index = OpenFile(srcFileName);
    if (index == -1)
        return makeError("ERR");

    // Create a copy of the fsInode object
    fsInode* copiedInode = new fsInode(it->second->getBlockSize());
    int newFileFD;

    // Insert the copied object with the new key
    MainDir[destFileName] = copiedInode;

    if (isOverRide)
    {
        FileDescriptor fd(destFileName, copiedInode);
        newFileFD = insertIntoVector(fd);
        if (newFileFD == -1)
            return makeError("ERR");
    }
    else
        newFileFD = OpenFile(destFileName);

    char data[DISK_SIZE];
    if (ReadFromFile(index, data, it->second->getFileSize()) == -1)
    {
        deleteFromMainDir(destFileName, false);
        CloseFile(index);
        return -1;
    }

    if (WriteToFile(newFileFD, data, it->second->getFileSize()) == -1)
    {
        CloseFile(newFileFD);
        deleteFromMainDir(destFileName, false);
        CloseFile(index);
        return -1;
    }

    CloseFile(newFileFD);
    CloseFile(index);
    return 1;
}

// ------------------------------------------------------------------------
int fsDisk::RenameFile(string oldFileName, string newFileName)
{
    if (!b_is_formated || !isInMap(oldFileName) || isInMap(newFileName) || oldFileName == newFileName)
        return makeError("ERR");

    int index = getFileDescriptor(oldFileName);
    if (index != -1 &&  openFileDescriptors[index].isInUse())
        return makeError("ERR");


    auto it = MainDir.find(oldFileName);

    // Retrieve the corresponding fsInode pointer.
    fsInode* inode = it->second;

    // Insert a new entry with the new filename and the same fsInode pointer.
    MainDir[newFileName] = inode;

    // Erase the old entry.
    MainDir.erase(it);

    if (index != -1)
        openFileDescriptors[index].setName(newFileName);

    // Return 1 to indicate success.
    return 1;
}

// Destructor
fsDisk::~fsDisk()
{
    fclose(sim_disk_fd);
    delete[] BitVector;

    // Delete all fsInode objects in the MainDir map
    for (auto &pair: MainDir)
        delete pair.second;

    // Iterate through the vector and delete each pointer
    for (std::vector<fsInode*>::iterator it = deletedFiles.begin(); it != deletedFiles.end(); ++it)
        delete *it; // Delete the pointed object

}

char fsDisk::decToBinaryChar(int n) {
    return static_cast<char>(n);
}