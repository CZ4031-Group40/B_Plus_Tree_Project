#include <algorithm>
#include <iostream>
#include <vector>
#include <queue>
#include "BPlusTree.h"
#include "Storage.h"
#include <set>

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

tuple<NBARecords *,int> BPlusTree::searchRangedRecord(float startKey, float endKey) {
    tuple<BPNode *, int> searchNodeReturned = searchNode(startKey);
    BPNode *firstNode = get<0>(searchNodeReturned);
    int no_of_node_accessed = get<1>(searchNodeReturned);
    auto *recordVectorPtr = new NBARecords();
    int i = 0;
    while (i < firstNode->keys.size()) {
        if (startKey == firstNode->keys[i]) {
            recordVectorPtr->records.insert(recordVectorPtr->records.end(),
                                      firstNode->recordPtrs[i]->records.begin(),
                                      firstNode->recordPtrs[i]->records.end());
            i++;
            break;
        }
        if (startKey < firstNode->keys[i] && firstNode->keys[i] < endKey) {
            recordVectorPtr->records.insert(recordVectorPtr->records.end(),
                                            firstNode->recordPtrs[i]->records.begin(),
                                            firstNode->recordPtrs[i]->records.end());
            i++;
            break;
        }
        i++;
    }

    if (i == firstNode->recordPtrs.size()) {
        firstNode = firstNode->nextLeaf;
        no_of_node_accessed++;
        i = 0;
    }

    while (firstNode != nullptr && firstNode->keys[i] <= endKey) {
        recordVectorPtr->records.insert(recordVectorPtr->records.end(),
                                  firstNode->recordPtrs[i]->records.begin(),
                                  firstNode->recordPtrs[i]->records.end());
        i++;
        if (i == firstNode->keys.size()) {
            firstNode = firstNode->nextLeaf;
            no_of_node_accessed++;
            i = 0;
        }
    }

    return tuple<NBARecords *,int>{recordVectorPtr, no_of_node_accessed+1};
}

// Helper function
tuple<BPNode *, int> BPlusTree::searchNode(float key) {
    int no_of_node_accessed = 0;
    if (root == nullptr) {
        return tuple<BPNode*, int>{nullptr,0};
    }
    BPNode *current = root;
    while (!current->isLeaf) {
        no_of_node_accessed++;
        for (int i = 0; i < current->keys.size(); i++) {
            if (key < current->keys[i]) {
                current = current->childNodePtrs[i];
                break;
            }

            if (i == current->keys.size() - 1) {
                current = current->childNodePtrs[i + 1];
                break;
            }
        }
    }
    return tuple<BPNode *, int>{current,no_of_node_accessed};
}

tuple<NBARecords *,int> BPlusTree::searchRecord(float key) {
    tuple<BPNode *, int> searchNodeReturned = searchNode(key);
    BPNode *foundNode = get<0>(searchNodeReturned);
    int no_of_node_accessed = get<1>(searchNodeReturned);
    for (int i = 0; i < foundNode->keys.size(); i++) {
        if (key == foundNode->keys[i]) {
            return tuple<NBARecords *,int>{foundNode->recordPtrs[i], no_of_node_accessed+1};
        }
    }

    return tuple<NBARecords *, int>{nullptr, no_of_node_accessed+1};
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

void BPlusTree::calculateStatistics(BPNode *current, int insert) {
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
        if(insert == 1) {
            cout << "Average Node Size in Bytes: " << (totalSize / totalNodes) << " bytes" << endl;
            cout << "Largest Node Size in Bytes: " << largestNodeSize << " bytes" << endl;
        }
    }
}


void BPlusTree::deleteRecord(float keyToDelete) {
    if (root == nullptr) {
        cout << "The tree is empty." << endl;
        return;
    }

    // Start the recursive delete process
    bool rootUnderflow = deleteRecordRecursively(root, keyToDelete);

    // If root becomes empty after deletion, update the tree structure
    if (root->keys.empty()) {
        BPNode* newRoot = root->childNodePtrs[0];
        delete root;
        root = newRoot;
    }
}

bool BPlusTree::deleteRecordRecursively(BPNode* currentNode, float keyToDelete) {
    if (currentNode->isLeaf) {
        // Handle deletion in a leaf node
        int indexToDelete = -1;
        for (int i = 0; i < currentNode->keys.size(); i++) {
            if (currentNode->keys[i] == keyToDelete) {
                indexToDelete = i;
                break;
            }
        }

        if (indexToDelete != -1) {
            currentNode->keys.erase(currentNode->keys.begin() + indexToDelete);
            currentNode->recordPtrs.erase(currentNode->recordPtrs.begin() + indexToDelete);
            currentNode->minKey =  !currentNode->keys.empty() ? currentNode->keys[0] : currentNode->minKey;
            // Check if the node becomes underflowed
            if (currentNode->keys.size() < (BPlusNodeSize + 1) / 2 && currentNode != root) {
                return true; // Node is underflowed, indicating that its parent needs adjustment
            }
        }

        return false; // Node is not underflowed
    } else {
        // Find the appropriate child node to traverse
        int childIndex = 0;
        while (childIndex < currentNode->keys.size() && keyToDelete >= currentNode->keys[childIndex]) {
            childIndex++;
        }

        // Recursively delete the record in the child node
        BPNode* childNode = currentNode->childNodePtrs[childIndex];
        bool childUnderflow = deleteRecordRecursively(childNode, keyToDelete);
        currentNode->minKey = currentNode->childNodePtrs[0]->minKey;
        if(childIndex>0)currentNode->keys[childIndex-1] = currentNode->childNodePtrs[childIndex]->minKey;

        if (childUnderflow) {
            // Handle underflow in the child node
            int leftChildIndex = (childIndex > 0) ? childIndex - 1 : -1;
            int rightChildIndex = (childIndex < currentNode->childNodePtrs.size() - 1) ? childIndex + 1 : -1;

            if(childNode->isLeaf){
                if (leftChildIndex != -1 && currentNode->childNodePtrs[leftChildIndex]->keys.size() - 1 >= (BPlusNodeSize + 1) / 2) {
                    // Redistribute with the left sibling
                    redistributeWithLeftSibling(currentNode, leftChildIndex, childIndex);
                    currentNode->keys[childIndex - 1] = currentNode->childNodePtrs[childIndex]->minKey;
                } else if (rightChildIndex != -1 && currentNode->childNodePtrs[rightChildIndex]->keys.size() - 1 >= (BPlusNodeSize + 1) / 2) {
                    // Redistribute with the right sibling
                    redistributeWithRightSibling(currentNode, childIndex, rightChildIndex);
                    if(childIndex>0) currentNode->keys[childIndex-1] = currentNode->childNodePtrs[childIndex]->minKey;
                    currentNode->keys[rightChildIndex-1] = currentNode->childNodePtrs[rightChildIndex]->minKey;
                } else {
                    // Merge with a neighboring child node if redistribution is not possible
                    mergeChildNodes(currentNode, childIndex, leftChildIndex, rightChildIndex);
                }
            }
            else{
                if (leftChildIndex != -1 && currentNode->childNodePtrs[leftChildIndex]->keys.size() - 1 >= BPlusNodeSize / 2) {
                    // Redistribute with the left sibling
                    redistributeWithLeftSibling(currentNode, leftChildIndex, childIndex);
                    currentNode->keys[childIndex - 1] = currentNode->childNodePtrs[childIndex]->minKey;
                } else if (rightChildIndex != -1 && currentNode->childNodePtrs[rightChildIndex]->keys.size() - 1 >= BPlusNodeSize  / 2) {
                    // Redistribute with the right sibling
                    redistributeWithRightSibling(currentNode, childIndex, rightChildIndex);
                    if(childIndex>0) currentNode->keys[childIndex-1] = currentNode->childNodePtrs[childIndex]->minKey;
                    currentNode->keys[rightChildIndex-1] = currentNode->childNodePtrs[rightChildIndex]->minKey;
                } else {
                    // Merge with a neighboring child node if redistribution is not possible
                    mergeChildNodes(currentNode, childIndex, leftChildIndex, rightChildIndex);
                }
            }
            currentNode->minKey = currentNode->childNodePtrs[0]->minKey;
            // Check if the parent node becomes underflowed
            if (currentNode->keys.size() < (BPlusNodeSize) / 2 && currentNode != root) {
                return true; // Node is underflowed, indicating that its parent needs adjustment
            }
        }
        return false; // Node is not underflowed
    }
}
void BPlusTree::redistributeWithLeftSibling(BPNode* parentNode, int leftChildIndex, int rightChildIndex) {
    BPNode* leftNode = parentNode->childNodePtrs[leftChildIndex];
    BPNode* rightNode = parentNode->childNodePtrs[rightChildIndex];

    if (leftNode->isLeaf){
        float borrowedKey = leftNode->keys.back();
        rightNode->keys.insert(rightNode->keys.begin(), borrowedKey);
        leftNode->keys.erase(leftNode->keys.end()-1);
        rightNode->minKey = rightNode->keys[0];
        NBARecords* borrowedRecord = leftNode->recordPtrs.back();
        rightNode->recordPtrs.insert(rightNode->recordPtrs.begin(), borrowedRecord);
        leftNode->recordPtrs.erase(leftNode->recordPtrs.end()-1);
    } else  {
        rightNode->keys.insert(rightNode->keys.begin(), rightNode->childNodePtrs[0]->minKey);
        leftNode->keys.erase(leftNode->keys.end()-1);
        // Move the corresponding child pointer if it's a non-leaf node
        BPNode* borrowedChild = leftNode->childNodePtrs.back();
        rightNode->childNodePtrs.insert(rightNode->childNodePtrs.begin(), borrowedChild);
        leftNode->childNodePtrs.erase(leftNode->childNodePtrs.end()-1);
        rightNode->keys[0] = rightNode->childNodePtrs[1]->minKey;
        rightNode->minKey = rightNode->childNodePtrs[0]->minKey;
    }
}

void BPlusTree::redistributeWithRightSibling(BPNode* parentNode, int leftChildIndex, int rightChildIndex) {
    BPNode* leftNode = parentNode->childNodePtrs[leftChildIndex];
    BPNode* rightNode = parentNode->childNodePtrs[rightChildIndex];

    // Move the leftmost key from the right sibling to the parent


    if (rightNode->isLeaf){
        float borrowedKey = rightNode->keys.front();
        leftNode->keys.insert(leftNode->keys.end(), borrowedKey);
        rightNode->keys.erase(rightNode->keys.begin());
        rightNode->minKey = rightNode->keys[0];

        NBARecords* borrowedRecord = rightNode->recordPtrs.front();
        leftNode->recordPtrs.insert(leftNode->recordPtrs.end(), borrowedRecord);
        rightNode->recordPtrs.erase(rightNode->recordPtrs.begin());
    } else  {
        leftNode->keys.insert(leftNode->keys.end(), rightNode->childNodePtrs[0]->minKey);
        rightNode->keys.erase(rightNode->keys.begin());
        // Move the corresponding child pointer if it's a non-leaf node
        BPNode* borrowedChild = rightNode->childNodePtrs.front();
        leftNode->childNodePtrs.insert(leftNode->childNodePtrs.end(), borrowedChild);
        rightNode->childNodePtrs.erase(rightNode->childNodePtrs.begin());
        rightNode->minKey = rightNode->childNodePtrs[0]->minKey;
    }
}

void BPlusTree::mergeChildNodes(BPNode* parentNode, int curChildIndex, int leftChildIndex, int rightChildIndex) {
    BPNode* leftNode = leftChildIndex != -1 ? parentNode->childNodePtrs[leftChildIndex] : nullptr;
    BPNode* rightNode = rightChildIndex != -1 ? parentNode->childNodePtrs[rightChildIndex] : nullptr;
    BPNode* curNode = parentNode->childNodePtrs[curChildIndex];

    if(leftNode != nullptr){

        if(curNode->isLeaf){
            // Move all keys and record pointers from the current node to the left node
            leftNode->keys.insert(leftNode->keys.end(), curNode->keys.begin(), curNode->keys.end());

            // Update the parent's keys and child pointers
            parentNode->keys.erase(parentNode->keys.begin() + curChildIndex-1);
            parentNode->childNodePtrs.erase(parentNode->childNodePtrs.begin() + curChildIndex);
            leftNode->recordPtrs.insert(leftNode->recordPtrs.end(), curNode->recordPtrs.begin(), curNode->recordPtrs.end());
            leftNode->nextLeaf = curNode->nextLeaf;
        }else{
            // Move all keys and record pointers from the current node to the left node
            leftNode->keys.insert(leftNode->keys.end(), curNode->childNodePtrs[0]->minKey);
            leftNode->keys.insert(leftNode->keys.end(), curNode->keys.begin(), curNode->keys.end());

            // Update the parent's keys and child pointers
            parentNode->keys.erase(parentNode->keys.begin() + curChildIndex-1);
            parentNode->childNodePtrs.erase(parentNode->childNodePtrs.begin() + curChildIndex);
            leftNode->childNodePtrs.insert(leftNode->childNodePtrs.end(), curNode->childNodePtrs.begin(), curNode->childNodePtrs.end());
        }
    } else {
        if(curNode->isLeaf){
            rightNode->keys.insert(rightNode->keys.begin(), curNode->keys.begin(), curNode->keys.end());

            // Update the parent's keys and child pointers
            parentNode->keys.erase(parentNode->keys.begin() + curChildIndex);
            parentNode->childNodePtrs.erase(parentNode->childNodePtrs.begin() + curChildIndex);
            rightNode->recordPtrs.insert(rightNode->recordPtrs.begin(), curNode->recordPtrs.begin(), curNode->recordPtrs.end());
            rightNode->minKey = rightNode->recordPtrs[0]->records[0]->homeFGPercentage;
        }else{
            rightNode->keys.insert(rightNode->keys.begin(), rightNode->childNodePtrs[0]->minKey);
            rightNode->keys.insert(rightNode->keys.begin(), curNode->keys.begin(), curNode->keys.end());

        // Update the parent's keys and child pointers
            parentNode->keys.erase(parentNode->keys.begin() + curChildIndex);
            parentNode->childNodePtrs.erase(parentNode->childNodePtrs.begin() + curChildIndex);
            rightNode->childNodePtrs.insert(rightNode->childNodePtrs.begin(), curNode->childNodePtrs.begin(), curNode->childNodePtrs.end());
            rightNode->minKey = rightNode->childNodePtrs[0]->minKey;
        }
    }

    delete curNode;
}


set<float> BPlusTree::searchRangedKeys(float startKey, float endKey) {
    set<float> uniqueKeys;

    tuple<BPNode*, int> searchNodeReturned = searchNode(startKey);
    BPNode* firstNode = get<0>(searchNodeReturned);

    int i = 0;
    while (firstNode != nullptr && firstNode->keys[i] <= endKey) {
        if (firstNode->keys[i] >= startKey) {
            uniqueKeys.insert(firstNode->keys[i]);
        }
        i++;

        if (i == firstNode->keys.size()) {
            firstNode = firstNode->nextLeaf;
            i = 0;
        }
    }

    return uniqueKeys;
}


BPNode *BPlusTree::getRoot() {
    return this->root;
}

BPlusTree::~BPlusTree() = default;