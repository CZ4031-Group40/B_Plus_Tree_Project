#ifndef B_PLUS_TREE_PROJECT_STORAGE_H
#define B_PLUS_TREE_PROJECT_STORAGE_H

using namespace std;

const unsigned int blockSize = 400;

struct NBARecord{
    unsigned int teamID; //4 bytes
    unsigned int homePoints; //4 bytes
    float homeFGPercentage; //4 bytes
    float homeFTPercentage; //4 bytes
    float homeFG3Percentage; //4 bytes
    unsigned int homeAssist; //4 bytes
    unsigned int homeRebound; //4 bytes
    char date[11];
    bool homeTeamWins; //1 byte
};

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
