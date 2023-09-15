#include <tuple>
#include <vector>
#ifndef B_PLUS_TREE_PROJECT_BPLUSTREE_H
#define B_PLUS_TREE_PROJECT_BPLUSTREE_H

using namespace std;

const unsigned int BPlusNodeSize = 10;

class BPNode{
private:
    bool isLeaf;
    int *key;
    int size;
    void **nodePtr;

public:
    BPNode();
};

class BPlusTree {
private:
    BPNode *root;

public:
    BPlusTree(); //for empty tree
    BPlusTree(vector<tuple<int , void * >> &initialData); // for bulk loading

    ~BPlusTree();

    void insertRecord();
    void deleteRecord();
    void* searchRecord(int key);
};


#endif //B_PLUS_TREE_PROJECT_BPLUSTREE_H
