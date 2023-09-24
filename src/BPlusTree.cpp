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
BPNode *BPlusTree::insertSplitRecord(float key, NBARecord* recordAddress, BPNode* curNode) {
    
    if (curNode->isLeaf) {
        
        unsigned int insertIdx = 0;
        while (insertIdx < curNode->keys.size()) {
            if (key == curNode->keys[insertIdx]) {
                // add to existing key
                NBARecords* recordVector = curNode->recordPtrs[insertIdx];
                recordVector->records.push_back(recordAddress);
                return nullptr;
            }
            if (key < curNode->keys[insertIdx]) {
                break;
            }
            insertIdx++;
        }

        if(curNode->keys.size() < curNode->size) {

            curNode->keys.insert(curNode->keys.begin()+insertIdx, key);
            auto *recordVector = new NBARecords();
            recordVector->records.push_back(recordAddress);
            curNode->recordPtrs.insert(curNode->recordPtrs.begin() + insertIdx, recordVector);   

        }
        // if current node exceeds capacity, create new leaf node and update parent by returning newNode
        else {

            //create new leaf node
            BPNode *newNode = new BPNode(true);

            auto *recordVector = new NBARecords();
            recordVector->records.push_back(recordAddress);

            curNode->keys.insert(curNode->keys.begin()+insertIdx, key);
            curNode->recordPtrs.insert(curNode->recordPtrs.begin() + insertIdx, recordVector);   

            int splitIdx = curNode->keys.size() / 2;

            // move keys from splitidx onwards from current to new node
            newNode->keys.assign(curNode->keys.begin() + splitIdx, curNode->keys.end());
            curNode->keys.erase(curNode->keys.begin() + splitIdx, curNode->keys.end());

            newNode->recordPtrs.assign(curNode->recordPtrs.begin() + splitIdx, curNode->recordPtrs.end());
            curNode->recordPtrs.erase(curNode->recordPtrs.begin() + splitIdx, curNode->recordPtrs.end());

            BPNode *curNextLeafNode = curNode->nextLeaf;
            // re-assign nextLeaf for the currentnode and the newnode
            curNode->nextLeaf = newNode;
            newNode->minKey = newNode->keys[0];
            newNode ->nextLeaf = curNextLeafNode;
            return newNode; // return newNode up the recursive call
        }

    }

    else {

        unsigned int insertIdx = 0;
        BPNode *childNode = nullptr;
        childNode = curNode->childNodePtrs[insertIdx];

        while (insertIdx < curNode->keys.size()) {
            
            if (key<curNode->childNodePtrs[insertIdx]) {
                break;
            }
            insertIdx++;
            childNode = curNode->childNodePtrs[insertIdx];
        }

        // call the function again on the child node
        BPNode *tempNode = nullptr;
        tempNode = insertSplitRecord(key, recordAddress, childNode);

        if (tempNode!=nullptr) {
            // new node was added somewhere below as tempNode is not null.
            // update current node to include that node.
            if (curNode->keys.size() < curNode->size) {

                curNode->keys.insert(curNode->keys.begin()+insertIdx, tempNode->keys[0]);
                curNode->childNodePtrs.insert(curNode->childNodePtrs.begin()+insertIdx+1, tempNode);
                
            }
            else {
                //create new non-leaf node
                BPNode *newNode = new BPNode(false);

                curNode->keys.insert(curNode->keys.begin()+insertIdx, key);
                curNode->childNodePtrs.insert(curNode->childNodePtrs.begin()+insertIdx+1, tempNode);

                int splitIdx = curNode->keys.size() / 2;

                // move keys from splitidx onwards from current to new node
                newNode->keys.assign(curNode->keys.begin() + splitIdx+1, curNode->keys.end());
                curNode->keys.erase(curNode->keys.begin() + splitIdx, curNode->keys.end());

                newNode->childNodePtrs.assign(curNode->childNodePtrs.begin() + splitIdx+1, curNode->childNodePtrs.end());
                curNode->childNodePtrs.erase(curNode->childNodePtrs.begin() + splitIdx+1, curNode->childNodePtrs.end());

                newNode->minKey = newNode->childNodePtrs[0]->minKey;

                return newNode;
            }
        }
    }
    return nullptr; 
}

void BPlusTree::insertRecord(float key, NBARecord* recordAddress) {
    
    BPNode *current = root;    

    if (current==nullptr) {
        return;
    }

    unsigned int insertIdx = 0;
    BPNode *childNode = nullptr;
    childNode = curNode->childNodePtrs[insertIdx];

    while (insertIdx < curNode->keys.size()) {
        
        if (key<curNode->childNodePtrs[insertIdx]) {
            break;
        }
        insertIdx++;
        childNode = curNode->childNodePtrs[insertIdx];
    }

    BPNode *tempNode = insertSplitRecord(key, recordAddress, childNode);

    if (tempNode!=nullptr) {
        if (current->keys.size() < current->size) {
            current->keys.insert(current->keys.begin()+insertIdx, tempNode->keys[0]);
            current->childNodePtrs.insert(current->childNodePtrs.begin()+insertIdx+1, tempNode);
        }
        else {
            BPNode *newNode = new BPNode(false);
            int splitIdx = (current->keys.size() / 2 );
            // move keys from splitidx onwards from current to new node
            newNode->keys.assign(current->keys.begin() + splitIdx+1, current->keys.end());
            current->keys.erase(current->keys.begin() + splitIdx, current->keys.end());
            
            newNode->childNodePtrs.assign(current->childNodePtrs.begin() + splitIdx+1, current->childNodePtrs.end());
            current->childNodePtrs.erase(current->childNodePtrs.begin() + splitIdx, current->childNodePtrs.end());

            newNode->minKey = newNode->childNodePtrs[0]->minKey
            ;
            BPNode *newRootNode = new BPNode(false);
            newRootNode->childNodePtrs.push_back(current);
            newRootNode->childNodePtrs.push_back(newNode);

            newRootNode->keys.push_back(newNode->minKey);

            this->root = newRootNode;
        }
    }    
    return;
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

