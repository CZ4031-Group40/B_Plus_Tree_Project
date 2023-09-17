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
};
#endif //B_PLUS_TREE_PROJECT_STORAGE_H
