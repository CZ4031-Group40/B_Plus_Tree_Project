#include <tuple>
#include <vector>
#include "DataStruct.h"
#ifndef B_PLUS_TREE_PROJECT_BPLUSTREE_H
#define B_PLUS_TREE_PROJECT_BPLUSTREE_H

using namespace std;

const unsigned int BPlusNodeSize = 36;

class BPNode{
    friend class BPlusTree;

private:
    int size = BPlusNodeSize; // 4 bytes
    bool isLeaf; // 1 bytes
    float minKey; // 4 byte
    BPNode *nextLeaf = nullptr; // 8 byte
    vector<float> keys; // 4 byte * number of keys
    vector<BPNode *> childNodePtrs; // 8 byte * number of child 
    vector<NBARecords *> recordPtrs; // 8 byte * number of records

public:
    BPNode(bool isLeaf);
    BPNode* getNextLeaf();
    vector<float> getKeys();
};

class BPlusTree {
private:
    BPNode *root;
    BPNode* insertSplitRecord(float recordKey, NBARecord* recordAddress, BPNode* curNode);

public:
    BPlusTree(); //for empty tree
    BPlusTree(vector<tuple<float , void * >> &initialData); // for bulk loading

    ~BPlusTree();

    void insertRecord(float recordKey, void* recordAddress);
    void deleteRecord();
    NBARecords* searchRecord(float key);
    void displayTree(BPNode *current);
    BPNode* getRoot();
    void calculateStatistics(BPNode *current);
    void displayRootNode();
    void getNodeSize();

};


#endif //B_PLUS_TREE_PROJECT_BPLUSTREE_H
