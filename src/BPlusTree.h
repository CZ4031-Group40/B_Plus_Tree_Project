#include <tuple>
#include <vector>
#include "DataStruct.h"
#ifndef B_PLUS_TREE_PROJECT_BPLUSTREE_H
#define B_PLUS_TREE_PROJECT_BPLUSTREE_H

using namespace std;

const unsigned int BPlusNodeSize = 2;

class BPNode{
    friend class BPlusTree;

private:
    int size = BPlusNodeSize;
    bool isLeaf;
    float minKey;
    BPNode *nextLeaf = nullptr;
    vector<float> keys;
    vector<BPNode *> childNodePtrs;
    vector<NBARecords *> recordPtrs;

public:
    BPNode(bool isLeaf);
    BPNode* getNextLeaf();
    vector<float> getKeys();
};

class BPlusTree {
private:
    BPNode *root;

public:
    BPlusTree(); //for empty tree
    BPlusTree(vector<tuple<float , void * >> &initialData); // for bulk loading

    ~BPlusTree();

    void insertRecord();
    void deleteRecord();
    void* searchRecord(int key);
    BPNode* getRoot();

};


#endif //B_PLUS_TREE_PROJECT_BPLUSTREE_H
