#include <algorithm>
#include <iostream>
#include <vector>
#include <queue>
#include "BPlusTree.h"
#include "Storage.h"

using namespace std;

BPNode::BPNode(bool isLeaf) {
    this->isLeaf = isLeaf;
    this->size = BPlusNodeSize;
}

BPNode *BPNode::getNextLeaf() {
    return this->nextLeaf;
}

vector<float> BPNode::getKeys() {
    return this->keys;
}

BPlusTree::BPlusTree() {
    this->root = nullptr;
}

BPlusTree::BPlusTree(vector<tuple<float, void *>> &initialData) {
    auto *bpNode = new BPNode(true);
    vector<BPNode *> bpNodes;

    //Pack the keys to leaf nodes
    for(auto &i : initialData){
        float recordKey = get<0>(i);
        auto recordAddress = static_cast<NBARecord *>(get<1>(i));

        if((bpNode->keys.size() < bpNode->size) || (bpNode->keys[bpNode->keys.size() - 1] * 1000 ==recordKey * 1000)){
            if(!bpNode->keys.empty() && bpNode->keys[bpNode->keys.size() - 1] == recordKey){
                NBARecords *recordVectorPtr = (bpNode->recordPtrs[bpNode->recordPtrs.size()-1]);
                recordVectorPtr->records.push_back(recordAddress);
            } else {
                auto *recordVector = new NBARecords();

                bpNode->keys.push_back(recordKey);
                recordVector->records.push_back(recordAddress);
                bpNode->recordPtrs.push_back(recordVector);
            }

        } else {
            auto *recordVector = new NBARecords();

            auto newNode = new BPNode(true);

            bpNodes.push_back(bpNode);
            bpNode = newNode;
            bpNode->keys.push_back(recordKey);
            recordVector->records.push_back(recordAddress);
            bpNode->recordPtrs.push_back(recordVector);
        }
    }

    //Make sure that element have at least floor[(n+1)/2]
    if(bpNode->keys.size() < (bpNode->size + 1) / 2){
        BPNode *lastNode = bpNodes[bpNodes.size()-1];
        int elementsToMove = lastNode->keys.size() - (lastNode->keys.size() + bpNode->keys.size()) / 2;

        bpNode->keys.insert(
                bpNode->keys.begin(),
                lastNode->keys.end() - elementsToMove,
                lastNode->keys.end()
        );
        bpNode->recordPtrs.insert(
                bpNode->recordPtrs.begin(),
                lastNode->recordPtrs.end() - elementsToMove,
                lastNode->recordPtrs.end()
        );

        lastNode->keys.resize(lastNode->keys.size() - elementsToMove);
        lastNode->recordPtrs.resize(lastNode->recordPtrs.size() - elementsToMove);
    }

    bpNodes.push_back(bpNode);
    for(unsigned int i=0; i<bpNodes.size(); i++){
        bpNodes[i]->nextLeaf = i<bpNodes.size()-1 ? bpNodes[i+1]: nullptr;
        bpNodes[i]->minKey = bpNodes[i]->keys[0];
    }

    //Merge parents recursively until only one node is left
    vector<BPNode*> parentNodes;
    while(bpNodes.size()>1){
        auto *parentNode = new BPNode(false);
        parentNode->childNodePtrs.push_back(bpNodes[0]);
        parentNode->minKey = bpNodes[0]->minKey;
        for(unsigned int i=1; i< bpNodes.size(); i++){
            if(parentNode->keys.size() < parentNode ->size){
                parentNode->childNodePtrs.push_back(bpNodes[i]);
                parentNode->keys.push_back(bpNodes[i]->minKey);
            } else {
                parentNode->minKey = parentNode->childNodePtrs[0]->minKey;
                cout<<parentNode->minKey<< " t" <<endl;
                parentNodes.push_back(parentNode);

                auto *newParentNode = new BPNode(false);
                parentNode=newParentNode;
                parentNode->childNodePtrs.push_back(bpNodes[i]);
                parentNode->minKey = bpNodes[i]->minKey;
            }
        }

        if(parentNode->keys.size() < parentNode->size/2 && !parentNodes.empty()){
            BPNode *lastNode = parentNodes[parentNodes.size()-1];
            int elementsToMove = lastNode->keys.size() - (lastNode->keys.size() + parentNode->keys.size()) / 2;

            parentNode->keys.insert(
                    parentNode->keys.begin(),
                    lastNode->keys.end() - elementsToMove,
                    lastNode->keys.end()
            );
            parentNode->childNodePtrs.insert(
                    parentNode->childNodePtrs.begin(),
                    lastNode->childNodePtrs.end() - elementsToMove,
                    lastNode->childNodePtrs.end()
            );
            for (int i = 0; i < parentNode->keys.size(); i++) {
                parentNode->keys[i] = parentNode->childNodePtrs[i+1]->minKey;
            }
            parentNode->minKey = parentNode->childNodePtrs[0]->minKey;
            lastNode->keys.resize(lastNode->keys.size() - elementsToMove);
            lastNode->childNodePtrs.resize(lastNode->childNodePtrs.size() - elementsToMove);
        }

        parentNodes.push_back(parentNode);
        bpNodes = parentNodes;

        vector<BPNode *> newParentNodes;
        parentNodes = newParentNodes;
    }

    this->root = bpNodes[0];
    
}

void BPlusTree::insertRecord() {

}

NBARecords *BPlusTree::searchRecord(float key) {
    if (root == nullptr) {
        return nullptr;
    }
    BPNode *current = root;
    while (!current->isLeaf) {
        for (int i = 0; i < current->keys.size(); i++) {
            if (key < current->keys[i]) {
                current = current->childNodePtrs[i];
                break;
            }

            if (i == current->keys.size()-1) {
                current = current->childNodePtrs[i+1];
                break;
            }
        }
    }

    for (int i = 0; i < current->keys.size(); i++) {
        cout << current->keys[i] << endl;
        if (key == current->keys[i]) {
            return current->recordPtrs[i];
        }
    }

    return nullptr;
}

void BPlusTree::displayTree(BPNode *current) {
    if (current == nullptr) {
        return;
    }
    int level = 1;
    queue<BPNode *> q;
    q.push(current);

    while (!q.empty()) {
        int l;
        l = q.size();
        cout << "Level " << level++ << ": ";

        for (int i = 0; i < l; i++) {
            BPNode *node = q.front();
            q.pop();

            for (int j = 0; j < node->keys.size(); j++) {
                cout << node->keys[j] << " ";
            }

            for (int j = 0; j < node->childNodePtrs.size(); j++) {
                if (node->childNodePtrs[j] != nullptr) {
                    q.push(node->childNodePtrs[j]);
                }
            }

            cout << "\t";
        }
        cout << endl;
        cout << endl;
    }


}

void BPlusTree::deleteRecord() {

}

BPNode *BPlusTree::getRoot() {
    return this->root;
}

BPlusTree::~BPlusTree() = default;

