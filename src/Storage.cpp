#include "Storage.h"
#include "BPlusTree.h"
#include "DataStruct.h"
#include <cstring>
#include <iostream>

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

unsigned int Storage::getNumOfAllocatedBlocks() const {
    return numOfAllocatedBlocks;
}

unsigned int Storage::getRecordsPerBlock() const {
    return blockSize / sizeof(NBARecord);
}

void *Storage::getStoragePtr() {
    return storagePtr;
}

//get currblockusedspace
unsigned int Storage::getCurrBlockUsedSpace() const {
    return currBlockUsedSpace;
}

unsigned int Storage::getBlockSize() const{
    return blockSize;
}

