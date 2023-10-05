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

    //Make sure that the last node have at least floor[(n+1)/2] elements except if it's the only node (means it is root)
    if(bpNode->keys.size() < (bpNode->size + 1) / 2 && bpNodes.size()>1){
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
            
            if (key < curNode->keys[insertIdx]) {
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

                curNode->keys.insert(curNode->keys.begin()+insertIdx, tempNode->minKey);
                curNode->childNodePtrs.insert(curNode->childNodePtrs.begin()+insertIdx+1, tempNode);
                
            }
            else {
                //create new non-leaf node
                BPNode *newNode = new BPNode(false);

                curNode->keys.insert(curNode->keys.begin()+insertIdx, tempNode->minKey);
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

void BPlusTree::insertRecord(float key, void* recordPtr) {
    
    auto recordAddress = static_cast<NBARecord *>(recordPtr);

    BPNode *current = root;    

    if (current==nullptr) {
        // Create a new root node and add the record
        BPNode *newRootNode = new BPNode(true);
        newRootNode->keys.push_back(key);

        // Create an NBARecords object for this key
        auto* recordVector = new NBARecords();
        recordVector->records.push_back(recordAddress);

        newRootNode->recordPtrs.push_back(recordVector);
        
        this->root = newRootNode;

        return;    
    }
    if (current->isLeaf) {
        //insert straight to root
        BPNode *tempNode = insertSplitRecord(key, recordAddress, current);
        
        // create a new root node with current and tempNode as the childnodes
        if (tempNode!=nullptr) {
            BPNode *newRootNode = new BPNode(false);
            newRootNode->childNodePtrs.push_back(current);
            newRootNode->childNodePtrs.push_back(tempNode);

            newRootNode->keys.push_back(tempNode->keys[0]);
            newRootNode->minKey = newRootNode->childNodePtrs[0]->minKey;

            this->root = newRootNode;
        }
        // it tempNode is null, record is already added to leaf node
    }
    else {
        unsigned int insertIdx = 0;
        BPNode *childNode = nullptr;
        childNode = current->childNodePtrs[insertIdx];

        while (insertIdx < current->keys.size()) {
            
            if (key<current->keys[insertIdx]) {
                break;
            }
            insertIdx++;
            childNode = current->childNodePtrs[insertIdx];
        }

        BPNode *tempNode = insertSplitRecord(key, recordAddress, childNode);
        
        if (tempNode!=nullptr) {
            if (current->keys.size() < current->size) {
                current->keys.insert(current->keys.begin()+insertIdx, tempNode->minKey);
                current->childNodePtrs.insert(current->childNodePtrs.begin()+insertIdx+1, tempNode);
            }
            else {
                BPNode *newNode = new BPNode(false);

                current->keys.insert(current->keys.begin()+insertIdx, tempNode->minKey);
                current->childNodePtrs.insert(current->childNodePtrs.begin()+insertIdx+1, tempNode);
                
                int splitIdx = (current->keys.size() / 2 );

                // move keys from splitidx onwards from current to new node
                newNode->keys.assign(current->keys.begin() + splitIdx+1, current->keys.end());
                current->keys.erase(current->keys.begin() + splitIdx, current->keys.end());
                
                newNode->childNodePtrs.assign(current->childNodePtrs.begin() + splitIdx+1, current->childNodePtrs.end());
                current->childNodePtrs.erase(current->childNodePtrs.begin() + splitIdx+1, current->childNodePtrs.end());

                newNode->minKey = newNode->childNodePtrs[0]->minKey;

                BPNode *newRootNode = new BPNode(false);
                newRootNode->childNodePtrs.push_back(current);
                newRootNode->childNodePtrs.push_back(newNode);

                newRootNode->keys.push_back(newNode->minKey);
                newRootNode->minKey = newRootNode->childNodePtrs[0]->minKey;

                this->root = newRootNode;
            }
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
            cout << "   " << " ";

            for (int j = 0; j < node->keys.size(); j++) {
                cout << "|" <<"";
                cout << node->keys[j] << " ";
                cout << "|" <<"";

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



void BPlusTree::displayRootNode() {
    if (root == nullptr) {
        cout << "The tree is empty." << endl;
        return;
    }

    cout << "Keys of root node:" << endl;

    for (int j = 0; j < root->keys.size(); j++) {
        cout << "|" <<"";
        cout << root->keys[j] << " ";
        cout << "|" <<"";

    }

    cout << "\t";
    cout << endl;
    cout << endl;

}

void BPlusTree::getNodeSize() {
    cout << "Node Size parameter, n: " << root->size << endl;
}

void BPlusTree::calculateStatistics(BPNode *current) {
    if (current == nullptr) {
        cout << "The tree is empty." << endl;
        return;
    }

    int totalNodes = 0;
    int totalSize = 0;
    int largestNodeSize = 0;

    queue<BPNode *> q;
    q.push(current);

    int level = 0;

    while (!q.empty()) {
        int nodesAtLevel = q.size();

        for (int i = 0; i < nodesAtLevel; i++) {
            BPNode *node = q.front();
            q.pop();

            // Calculate the size of the node dynamically based on its components
            int nodeSize = sizeof(*node)+
                           sizeof(float) * node->keys.size() +
                           sizeof(BPNode *) * node->childNodePtrs.size() +
                           sizeof(NBARecords *) * node->recordPtrs.size();

            totalSize += nodeSize;
            totalNodes++;

            if (nodeSize > largestNodeSize) {
                largestNodeSize = nodeSize;
            }

            for (int j = 0; j < node->childNodePtrs.size(); j++) {
                if (node->childNodePtrs[j] != nullptr) {
                    q.push(node->childNodePtrs[j]);
                }
            }
        }

        level++;
    }

    if (level > 0) {
        cout << "Total Levels: " << level << endl;
        cout << "Total Nodes: " << totalNodes << endl;
        cout << "Average Node Size in Bytes: " << (totalSize / totalNodes) << " bytes" << endl;
        cout << "Largest Node Size in Bytes: " << largestNodeSize << " bytes" << endl;
    }
}

void BPlusTree::deleteRecord(float key){
    if (root == nullptr) return; // tree is empty, nothing to delete
    BPNode *current = root;

    vector<BPNode *> path;
    path.push_back(current);

    // 1. Remove the leaf node
    while (!current->isLeaf) {
        for (int i = 0; i < current->keys.size(); i++) {
            if (key < current->keys[i]) {
                current = current->childNodePtrs[i];
                path.push_back(current);
                break;
            }

            if (i == current->keys.size()-1) {
                current = current->childNodePtrs[i+1];
                path.push_back(current);
                break;
            }
        }
    }

    float replacementKey = current->keys[0];
    cout << path.size() << endl;

    bool found = false;

    for (int i = 0; i < current->keys.size(); i++) {
        if (key == current->keys[i]) {
            current->keys.erase(current->keys.begin()+i);
            current->recordPtrs.erase(current->recordPtrs.begin()+i);
            current->minKey = current->keys[0];
            replacementKey = current->keys[i]; // get the new key in its place
            found = true;
            break;
        }
    }

    if (!found) {
//        cout << "key not found" << endl;
        return;
    }

    // 1b: check all parents for deleted key - replace with replacement

    for (int i = path.size()-2; i >= 0; i--) {
        BPNode *parent = path[i];
//        cout << "key: " << key << endl;
//        cout <<  "level " << i << "; parent key: " << parent->keys[0] << endl;
        for (float & j : parent->keys) {
            if (key == j) {
//                cout << "key found in parent" << endl;
                j = replacementKey;
                parent->minKey = parent->childNodePtrs[0]->minKey;
                break;
            }
        }
    }

    // 2. If the leaf ends up with fewer than L/2 - underflow
    int minLeafSize;
    // check for lead underflow
    for (int i = path.size()-1; i > 0; i--) {
        current = path[i];
        if (i == path.size()-1) {
            minLeafSize = (BPlusNodeSize + 1) / 2;
        }
        else {
            minLeafSize = BPlusNodeSize / 2;
        }
        int leafSize = static_cast<int>(current->keys.size());
        BPNode *parent = path[i - 1];
        int currentIdx = 0;
        for (int j = 0; j < parent->childNodePtrs.size(); j++) {
            if (parent->childNodePtrs[j] == current) {
                currentIdx = j;
                break;
            }
        }
        if (leafSize < minLeafSize) {
    //        cout << "leaf underflow" << endl;
            // 2a. Adopt data from a neighbour, update the parent

            // find the index of the current node in the parent

            // find the left and right neighbours of the current node
            BPNode *leftNeighbour = nullptr;
            BPNode *rightNeighbour = nullptr;
            if (currentIdx > 0) {
                leftNeighbour = parent->childNodePtrs[currentIdx - 1];
            }
            if (currentIdx < parent->childNodePtrs.size() - 1) {
                rightNeighbour = parent->childNodePtrs[currentIdx + 1];
            }
            // if the left neighbour has more than L/2 elements, adopt from the left neighbour
            if (leftNeighbour != nullptr && leftNeighbour->keys.size() > minLeafSize) {

                // debug
    //            cout << "value of currentIdx: " << currentIdx << endl;
    //            cout << "parent key: " << parent->keys[currentIdx - 1] << endl;
    //            cout << "parent key size: " << parent->keys.size() << endl;
    //            cout << "current value: " << parent->keys[currentIdx - 1] << endl;
    //            cout << "value to be updated: " << leftNeighbour->keys[leftNeighbour->keys.size() - 1] << endl;
                // end debug

                // print all keys in the parent
    //            cout << "parent keys: ";
                for (float pKey: parent->keys) {
    //                cout << pKey << " ";
                }
    //            cout << endl;
    //            cout << "adopt from left neighbour" << endl;
                // adopt the last element from the left neighbour
                current->keys.insert(current->keys.begin(), leftNeighbour->keys[leftNeighbour->keys.size() - 1]);
                if (current->isLeaf) {
                    current->recordPtrs.insert(current->recordPtrs.begin(),
                                               leftNeighbour->recordPtrs[leftNeighbour->recordPtrs.size() - 1]);
                    current->minKey = current->keys[0];
                }
                else {
                    current->childNodePtrs.insert(current->childNodePtrs.begin(),
                                                  leftNeighbour->childNodePtrs[leftNeighbour->childNodePtrs.size() - 1]);
                    current->keys[0]= current->childNodePtrs[1]->minKey;
                    current->minKey = current->childNodePtrs[0]->minKey;
                }
                // update the parent - not working
    //            cout << "parent key before: " << parent->keys[currentIdx - 1] << endl;
                if (current->isLeaf) {
                    parent->keys[currentIdx - 1] = leftNeighbour->keys[leftNeighbour->keys.size() - 1];
                }
                else {
//                    parent->keys[currentIdx - 1] = leftNeighbour->childNodePtrs[0]->minKey;
                    parent->keys[currentIdx - 1] = current->minKey;
                }
                parent->minKey = parent->childNodePtrs[0]->minKey;
    //            cout << "parent key after: " << parent->keys[currentIdx - 1] << endl;

                // remove the adopted element from the left neighbour
                leftNeighbour->keys.erase(leftNeighbour->keys.end() - 1);
                if (current->isLeaf) {
                    leftNeighbour->recordPtrs.erase(leftNeighbour->recordPtrs.end() - 1);
                    leftNeighbour->minKey = leftNeighbour->keys[0];
                }
                else {
                    leftNeighbour->childNodePtrs.erase(leftNeighbour->childNodePtrs.end() - 1);
                }
            }

                // if the right neighbour has more than L/2 elements, adopt from the right neighbour
            else if (rightNeighbour != nullptr && rightNeighbour->keys.size() > minLeafSize) {
                // print all keys in the parent
    //            cout << "parent keys: ";
                for (float pKey: parent->keys) {
//                    cout << pKey << " ";
                }
                cout << endl;

    //            cout << "adopt from right neighbour" << endl;
                // adopt the first element from the right neighbour

                current->keys.push_back(rightNeighbour->keys[0]);
                if (current->isLeaf) {
                    current->recordPtrs.push_back(rightNeighbour->recordPtrs[0]);
                    current->minKey = current->keys[0];
                }
                else {
                    current->childNodePtrs.push_back(rightNeighbour->childNodePtrs[0]);
                    current->keys[current->keys.size()-1] = rightNeighbour->minKey;
                }

                // update the parent
    //            cout << "parent key before: " << parent->keys[currentIdx] << endl;
                if (current->isLeaf) {
                    parent->keys[currentIdx] = rightNeighbour->keys[1];
                    parent->minKey = parent->childNodePtrs[0]->minKey;
                }
    //            cout << "parent key after: " << parent->keys[currentIdx] << endl;

                // remove the adopted element from the right neighbour
                rightNeighbour->keys.erase(rightNeighbour->keys.begin());
                if (current->isLeaf) {
                    rightNeighbour->recordPtrs.erase(rightNeighbour->recordPtrs.begin());
                    rightNeighbour->minKey = rightNeighbour->keys[0];
                }
                else {
                    rightNeighbour->childNodePtrs.erase(rightNeighbour->childNodePtrs.begin());
                    rightNeighbour->minKey = rightNeighbour->childNodePtrs[0]->minKey;
                }

                if (!current->isLeaf) {
                    parent->keys[currentIdx] = rightNeighbour->minKey;
                }
            }

                // if adoption won't work, merge with neighbour - may result in parent underflow
            else {
    //            cout << "merge with neighbour" << endl;
                // merge with the left neighbour
                if (leftNeighbour != nullptr) {
                    // move all elements from the current node to the left neighbour
                    if (current->isLeaf) {
                        leftNeighbour->recordPtrs.insert(leftNeighbour->recordPtrs.end(), current->recordPtrs.begin(),
                                                         current->recordPtrs.end());
                    }
                    else {
                        leftNeighbour->childNodePtrs.insert(leftNeighbour->childNodePtrs.end(), current->childNodePtrs.begin(),
                                                            current->childNodePtrs.end());
                        current->keys.push_back(current->minKey);
                    }
                    leftNeighbour->keys.insert(leftNeighbour->keys.end(), current->keys.begin(), current->keys.end());
                    leftNeighbour->minKey = leftNeighbour->keys[0];
                    // update the parent
                    parent->keys.erase(parent->keys.begin() + currentIdx - 1);
                    parent->childNodePtrs.erase(parent->childNodePtrs.begin() + currentIdx);
                    parent->minKey = parent->childNodePtrs[0]->minKey;

                    // update the nextLeaf pointer
                    leftNeighbour->nextLeaf = current->nextLeaf;

                    // delete the current node
                    delete current;
                }

                    // merge with the right neighbour
                else if (rightNeighbour != nullptr) {
                    // move all elements from the right neighbour to the current node

                    if (current->isLeaf) {
                        current->recordPtrs.insert(current->recordPtrs.end(), rightNeighbour->recordPtrs.begin(),
                                                   rightNeighbour->recordPtrs.end());
                        current->keys.insert(current->keys.end(), rightNeighbour->keys.begin(), rightNeighbour->keys.end());
                        current->minKey = current->keys[0];
                    }
                    else {
                        current->childNodePtrs.insert(current->childNodePtrs.end(), rightNeighbour->childNodePtrs.begin(),
                                                     rightNeighbour->childNodePtrs.end());
                        current->keys[current->keys.size()-1] = rightNeighbour->childNodePtrs[0]->minKey;
                        current->keys.insert(current->keys.end(), rightNeighbour->keys.begin(), rightNeighbour->keys.end());
                    }

                    // update the parent
                    parent->keys.erase(parent->keys.begin() + currentIdx);
                    parent->childNodePtrs.erase(parent->childNodePtrs.begin() + currentIdx + 1);
                    parent->minKey = parent->childNodePtrs[0]->minKey;

                    // update the nextLeaf pointer
                    current->nextLeaf = rightNeighbour->nextLeaf;

                    // delete the right neighbour
                    delete rightNeighbour;
                }
            }
    }

        else {
            parent->keys[currentIdx - 1] = parent->childNodePtrs[currentIdx]->minKey;
            parent->minKey = parent->childNodePtrs[0]->minKey;
        }

        if (root->keys.empty() && !root->childNodePtrs.empty()) {
            root = root->childNodePtrs[0];
        }
    }
};

BPNode *BPlusTree::getRoot() {
    return this->root;
}

BPlusTree::~BPlusTree() = default;

