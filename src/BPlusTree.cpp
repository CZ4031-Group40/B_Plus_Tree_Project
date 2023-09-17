#include <iostream>
#include <vector>
#include "BPlusTree.h"
#include "Storage.h"

using namespace std;

BPNode::BPNode(bool isLeaf) {
    this->isLeaf = isLeaf;
    this->size = BPlusNodeSize;
}

BPlusTree::BPlusTree() {
    this->root = nullptr;
}

BPlusTree::BPlusTree(vector<tuple<float, void *>> &initialData) {
    //test
    BPNode bpNode(true);
    vector<BPNode> bpNodes;

    //Pack the keys to leaf nodes
    for(auto &i : initialData){
        float recordKey = get<0>(i);
        auto recordAddress = static_cast<NBARecord *>(get<1>(i));

        if((bpNode.keys.size() < bpNode.size) || (bpNode.keys[bpNode.keys.size() - 1] * 1000 ==recordKey * 1000)){
            if(!bpNode.keys.empty() && bpNode.keys[bpNode.keys.size() - 1] == recordKey){
                vector<NBARecord *> *recordVectorPtr = &(bpNode.recordPtrs[bpNode.recordPtrs.size()-1]);
                recordVectorPtr->push_back(recordAddress);
            } else {
                vector<NBARecord *> recordVector;

                bpNode.keys.push_back(recordKey);
                recordVector.push_back(recordAddress);
                bpNode.recordPtrs.push_back(recordVector);
            }

        } else {
            vector<NBARecord *> recordVector;
            BPNode newNode = BPNode(true);

            bpNodes.push_back(bpNode);
            bpNode = newNode;
            bpNode.keys.push_back(recordKey);
            recordVector.push_back(recordAddress);
            bpNode.recordPtrs.push_back(recordVector);
        }
    }


//    Make sure that element have at least floor[(n+1)/2]
    if(bpNode.keys.size() < (bpNode.size + 1) / 2){
        BPNode *lastNode = &bpNodes[bpNodes.size()-1];
        int elementsToMove = lastNode->keys.size() - (lastNode->keys.size() + bpNode.keys.size()) / 2;

        bpNode.keys.insert(
                bpNode.keys.begin(),
                lastNode->keys.end() - elementsToMove,
                lastNode->keys.end()
        );
        bpNode.recordPtrs.insert(
                bpNode.recordPtrs.begin(),
                lastNode->recordPtrs.end() - elementsToMove,
                lastNode->recordPtrs.end()
        );

        lastNode->keys.resize(lastNode->keys.size() - elementsToMove);
        lastNode->recordPtrs.resize(lastNode->recordPtrs.size() - elementsToMove);
    }

    bpNodes.push_back(bpNode);

    for(auto node:bpNodes){
        cout<<node.keys.size()<<" ";
        for(unsigned int i=0; i<node.keys.size(); i++){
            cout<<node.keys[i]<< " ";
            cout<<node.recordPtrs[i].size()<< " ";
        }
        cout << endl;
    }

}

void BPlusTree::insertRecord() {

}

void* BPlusTree::searchRecord(int key) {
 return nullptr;
}

void BPlusTree::deleteRecord() {

}

BPlusTree::~BPlusTree() = default;

