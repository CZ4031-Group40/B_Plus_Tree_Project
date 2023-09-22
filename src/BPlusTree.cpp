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

            parentNode->childNodePtrs.insert(
                    parentNode->childNodePtrs.begin(),
                    lastNode->childNodePtrs.end() - elementsToMove,
                    lastNode->childNodePtrs.end()
            );
            for (unsigned int i = elementsToMove; i > 0; i--) {
                parentNode->keys.insert(
                        parentNode->keys.begin(),
                        parentNode->childNodePtrs[i]->minKey
                );
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

void BPlusTree::insertRecord(float recordKey, NBARecord* recordAddress) {
    // Check if the tree is empty
    if (root == nullptr) {
        // Create a new root node and add the record
        root = new BPNode(true);
        root->keys.push_back(recordKey);

        // Create an NBARecords object for this key
        auto* recordVector = new NBARecords();
        recordVector->records.push_back(recordAddress);

        root->recordPtrs.push_back(recordVector);
        return;
    }

    queue<BPNode *> q; //this maintains the list of nodes traversed down for parent node merging
    BPNode *current = root;
    while (!current->isLeaf) { 
        q.push(current);
        // find target leaf node
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
    // at target leaf node, find location to insert
    for (int j = 0; i < current->keys.size(); j++) {
        if (key == current->keys[j]) { 
            // add record with duplicate key
            NBARecords *recordVector = current->recordPtrs[j];
            recordVector->records.push_back(recordAddress);

            return;
        }
        if (key < current->keys[j]) {
            // insert the key and record at j
            current->keys.insert(current->keys.begin()+j, key );

            // Create an NBARecords object for this key
            auto *recordVector = new NBARecords();
            recordVector->records.push_back(recordAddress);

            current->recordPtrs.insert(current->recordPtrs.begin() + j, recordVector);      

            // if current node exceeds capacity, create new leaf node and update parent
            if(current->keys.size() > current->size) {
                //create new leaf node
                BPNode *newNode = new BPNode(true);
                int splitIdx = current->keys.size() / 2;

                // move keys from splitidx onwards from current to new node
                current->keys.erase(current->keys.begin() + splitIndex, current->keys.end());
                newNode->keys.assign(current->keys.begin() + splitIndex, current->keys.end());

                // move ptrs from splitidx onwards from current to new node
                current->recordPtrs.erase(current->recordPtrs.begin() + splitIndex, current->recordPtrs.end());
                newNode->recordPtrs.assign(current->recordPtrs.begin() + splitIndex, current->recordPtrs.end());

                BPNode *curNextLeafNode = current->nextLeaf;
                // re-assign nextLeaf for the currentnode and the newnode
                current->nextLeaf = newNode;
                newNode ->nextLeaf = curNextLeafNode;

                // Adjust parent pointers if necessary
                if (current == root) {
                    // Create a new root node and make it point to current and newNode
                    root = new BPNode(false); // The new root is not a leaf node
                    root->keys.push_back(newNode->keys[0]);
                    root->childNodePtrs.push_back(current);
                    root->childNodePtrs.push_back(newNode);
                } else {
                    while (!q.empty()){
                        BPNode *parent = q.back(); // get the parent node from queue
                        q.pop();
                        int idxInParent = 0;
                        while (indexInParent < parent->keys.size() && newNode->keys[0] > parent->keys[indexInParent]) {
                            indexInParent++;
                        }
                        if (parent->keys.size() < parent->size) {
                            // parent node still have space, 
                            //add new leaf node to parent node and return
                            parent->keys.insert(parent->keys.begin() + indexInParent, newNode->keys[0]);
                            parent->childNodePtrs.insert(parent->childNodePtrs.begin() + indexInParent + 1, newNode);
                            return;
                        }
                        else {
                            //parent node has no space,
                            //split parent node
                            auto *newParent = new BPNode(false);
                            


                        }
                    }

                }
            }
        }
    }


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

