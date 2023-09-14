#include "Storage.h"
#include <cstring>

using namespace std;

Storage::Storage(unsigned int storageSize, unsigned int blockSize) {
    this->storageSize = storageSize;
    this->blockSize = blockSize;

    storagePtr = new unsigned char[storageSize];
    currBlockPtr = nullptr;

    numOfBlocks = storageSize / blockSize;
    numOfAllocatedBlocks = 0;
    currBlockUsedSpace = 0;
}

Storage::~Storage() {
    delete storagePtr;
}

void *Storage::storeRecord(NBARecord record) {
    if(blockSize - currBlockUsedSpace <= sizeof(record) || numOfAllocatedBlocks == 0){
        if(numOfAllocatedBlocks < numOfBlocks ){
            currBlockPtr = storagePtr + numOfAllocatedBlocks * blockSize;
            numOfAllocatedBlocks += 1;
            currBlockUsedSpace = 0;
        } else {
            throw "There are no available space";
        }
    }

    void *recordPtr = currBlockPtr + currBlockUsedSpace;
    currBlockUsedSpace += sizeof(record);
    memcpy(recordPtr, &record, sizeof(record));
    return recordPtr;
}