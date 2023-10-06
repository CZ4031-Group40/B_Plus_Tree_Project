#include "BPlusTree.h"
#include "DataStruct.h"
#ifndef B_PLUS_TREE_PROJECT_STORAGE_H
#define B_PLUS_TREE_PROJECT_STORAGE_H

using namespace std;

const unsigned int blockSize = 400;

class Storage{
private:
    unsigned char *storagePtr;
    unsigned char *currBlockPtr;

    unsigned int storageSize;
    unsigned int blockSize;
    unsigned int numOfBlocks;
    unsigned int numOfAllocatedBlocks;
    unsigned int currBlockUsedSpace;

public:
    Storage(unsigned int storageSize, unsigned int blockSize);

    ~Storage();

    void* storeRecord(NBARecord record);
    unsigned int getNumOfAllocatedBlocks() const;
    unsigned int getRecordsPerBlock() const;
    void* getStoragePtr();
    unsigned int getCurrBlockUsedSpace() const;

    unsigned int getBlockSize();

    unsigned int getBlockSize() const;
};
#endif //B_PLUS_TREE_PROJECT_STORAGE_H
