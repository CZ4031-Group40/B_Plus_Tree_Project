#include <algorithm>
#include <cmath>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <set>
#include "BPlusTree.h"
#include "Storage.h"

using namespace std;

int main() {
    cout << "========================================EXPERIMENT ONE========================================" << endl;

    Storage storage{static_cast<unsigned int>(100 * pow(10, 6)), blockSize};

    ifstream allData("../data/games.txt");

    string line;

    vector<tuple<float, void*>> recordPtrs;
    vector<int> errorRows; // Store row numbers with errors
    int count = 0;
    getline(allData, line);
    while (getline(allData, line)) {
        count += 1;
        istringstream iss(line);
        NBARecord newRecord{};

        if (!(iss >> newRecord.date) ||
            !(iss >> newRecord.teamID) ||
            !(iss >> newRecord.homePoints) ||
            !(iss >> newRecord.homeFGPercentage) ||
            !(iss >> newRecord.homeFTPercentage) ||
            !(iss >> newRecord.homeFG3Percentage) ||
            !(iss >> newRecord.homeAssist) ||
            !(iss >> newRecord.homeRebound) ||
            !(iss >> newRecord.homeTeamWins)) {
            errorRows.push_back(count); 
            continue; 
        }

        void* recordPtr = storage.storeRecord(newRecord);
        recordPtrs.push_back(tuple<float, void*>{newRecord.homeFGPercentage, recordPtr});
    }
    allData.close();

    cout << "========================================" << endl;
    unsigned int numOfRecords = recordPtrs.size();
    cout << "Length of recordPtrs: " << numOfRecords << endl;

    cout << "========================================" << endl;
    size_t recordSize = sizeof(NBARecord);
    cout << "Size of a record: " << recordSize << " bytes" << endl;

    cout << "========================================" << endl;
    unsigned int allocatedBlocks = storage.getNumOfAllocatedBlocks();
    cout << "Current number of blocks used to store the data: " << allocatedBlocks << endl;

    cout << "========================================" << endl;
    float avgRecordsPerBlock = static_cast<float>(numOfRecords) / allocatedBlocks;
    cout << "Average number of records per block : " << avgRecordsPerBlock << endl;

    cout << "========================================" << endl;
    // Print row numbers with errors at the end
    if (!errorRows.empty()) {
        // cout << "Rows with errors:";
        // for (int row : errorRows) {
        //     cout << " " << row << ",";
        // }
        // cout << endl;
    cout << "Number of invalid records: " << errorRows.size() << endl;

    }

    cout << "========================================EXPERIMENT TWO========================================" << endl;

    BPlusTree bPlusTree; // Create an empty B+ tree

    for (auto it = recordPtrs.begin(); it < recordPtrs.end(); it++) {
        float homeFGPercentage = get<0>(*it);
        void* recordPtr = get<1>(*it);

        // Insert the record into the bPlusTree
        bPlusTree.insertRecord(homeFGPercentage, recordPtr);
    }
    BPNode *ptr=bPlusTree.getRoot();
    bPlusTree.getNodeSize();
    bPlusTree.calculateStatistics(ptr,1);
    // bPlusTree.displayTree(ptr);

    bPlusTree.displayRootNode();


    cout << "=======================================EXPERIMENT 3====================================" << endl;

    float queriedFGP = 0.5;
    cout << "Searching for FG_PCT home = " << queriedFGP << endl;

    // Calculate running time
    auto start = chrono::high_resolution_clock::now();
    tuple<NBARecords *, int> queriedRecords = bPlusTree.searchRecord(queriedFGP);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> time_taken = end - start;

    NBARecords *queriedData = get<0>(queriedRecords);
    int no_of_node_accessed = get<1>(queriedRecords);

    float FG3_total = 0;
    float FG3_average = 0;

    if (queriedData == nullptr) {
        cout << "Can't find record" << endl;
    }
    else {
        for (int i = 0; i < queriedData->records.size(); i++) {
            NBARecord *record = queriedData->records[i];
            FG3_total += record->homeFG3Percentage;
        }
        FG3_average = FG3_total/queriedData->records.size();
    }
    int recordsPerBlock = storage.getRecordsPerBlock();
    int numberOfRecordsRetrieved = queriedData->records.size();
    int no_of_blocks_accessed = no_of_node_accessed + static_cast<int>(ceil(numberOfRecordsRetrieved/recordsPerBlock));

    start = chrono::high_resolution_clock::now();
    auto *queriedData2 = new NBARecords();
    auto *storagePtr = static_cast<unsigned char*>(storage.getStoragePtr());
    void *curBlockPtr;
    void *curRecordPtr;
    unsigned int numOfAllocatedBlocks = storage.getNumOfAllocatedBlocks();
    for (int i = 0; i < numOfAllocatedBlocks; i++) {
        curBlockPtr = storagePtr + (i * blockSize);
        for (int j = 0; j < recordsPerBlock; j++) {
            curRecordPtr = static_cast<unsigned char*>(curBlockPtr) + (j * recordSize);
            auto *curNBARecord = static_cast<NBARecord *>(curRecordPtr);
            if (curNBARecord->homeFGPercentage == queriedFGP) {
                queriedData2->records.push_back(curNBARecord);
            }
        }
    }
    end = chrono::high_resolution_clock::now();
    chrono::duration<double> time_taken_linear_scan = end - start;

    cout << "Number of index node accessed: " << no_of_node_accessed << endl;
    cout << "=======================================================================================" << endl;
    cout << "Number of data blocks accessed: " << no_of_blocks_accessed << endl;
    cout << "=======================================================================================" << endl;
    cout << "The average of \"FG3_PCT_home\" of the records that are returned: " << FG3_average << endl;
    cout << "=======================================================================================" << endl;
    cout << "The running time of the retrieval process: " << time_taken.count()*1000 << "ms"  << endl;
    cout << "=======================================================================================" << endl;
    cout << "The number of data blocks that would be accessed by a brute-force linear scan method: " << storage.getNumOfAllocatedBlocks() << endl;
    cout << "=======================================================================================" << endl;
    cout << "The running time (Linear Scan): " << time_taken_linear_scan.count()*1000 << "ms"  << endl;
    cout << endl;


    cout << "=======================================EXPERIMENT 4====================================" << endl;

    float startKey = 0.6;
    float endKey = 1;
    cout << "Searching for FG_PCT home between " << startKey << " and " << endKey  << endl;

    // Calculate running time
    start = chrono::high_resolution_clock::now();
    queriedRecords = bPlusTree.searchRangedRecord(startKey, endKey);
    end = chrono::high_resolution_clock::now();
    time_taken = end - start;
    queriedData = get<0>(queriedRecords);
    no_of_node_accessed = get<1>(queriedRecords);
    FG3_total = 0;
    FG3_average = 0;

    if (queriedData == nullptr) {
        cout << "Can't find record" << endl;
    }
    else {
        for (int i = 0; i < queriedData->records.size(); i++) {
            NBARecord *record = queriedData->records[i];
            FG3_total += record->homeFG3Percentage;
        }
        FG3_average = FG3_total/queriedData->records.size();
    }
    recordsPerBlock = storage.getRecordsPerBlock();
    numberOfRecordsRetrieved = queriedData->records.size();
    no_of_blocks_accessed = no_of_node_accessed + static_cast<int>(ceil(numberOfRecordsRetrieved/recordsPerBlock));

    start = chrono::high_resolution_clock::now();
    queriedData2 = new NBARecords();
    storagePtr = static_cast<unsigned char*>(storage.getStoragePtr());
    recordSize = sizeof(NBARecord);
    numOfAllocatedBlocks = storage.getNumOfAllocatedBlocks();
    for (int i = 0; i < numOfAllocatedBlocks; i++) {
        curBlockPtr = storagePtr + (i * blockSize);
        for (int j = 0; j < recordsPerBlock; j++) {
            curRecordPtr = static_cast<unsigned char*>(curBlockPtr) + (j * recordSize);
            auto *curNBARecord = static_cast<NBARecord *>(curRecordPtr);
            if (curNBARecord->homeFGPercentage >= startKey && curNBARecord->homeFGPercentage <= endKey) {
                queriedData2->records.push_back(curNBARecord);
            }
        }
    }
    end = chrono::high_resolution_clock::now();
    time_taken_linear_scan = end - start;
    cout << "Number of index node accessed: " << no_of_node_accessed << endl;
    cout << "=======================================================================================" << endl;
    cout << "Number of data blocks accessed: " << no_of_blocks_accessed << endl;
    cout << "=======================================================================================" << endl;
    cout << "The average of \"FG3_PCT_home\" of the records that are returned: " << FG3_average << endl;
    cout << "=======================================================================================" << endl;
    cout << "The running time of the retrieval process: " << time_taken.count()*1000 << "ms" << endl;
    cout << "=======================================================================================" << endl;
    cout << "The number of data blocks that would be accessed by a brute-force linear scan method: " << storage.getNumOfAllocatedBlocks() << endl;
    cout << "=======================================================================================" << endl;
    cout << "The running time (Linear Scan): " << time_taken_linear_scan.count()*1000 << "ms" << endl;
    cout << endl;


    cout << "========================================EXPERIMENT FIVE========================================" << endl;
    BPlusTree bPlusTreeLS = bPlusTree;

    auto startDelete = chrono::high_resolution_clock::now();

    tuple<NBARecords *, int> result;
    result = bPlusTree.searchRangedRecord(0,0.35);
    NBARecords *resultData = get<0>(result);


    int deleteCount = 0;

    set<float> distinctHomeFGPercentages;

    for (int i = 0; i < resultData->records.size(); i++) {
        NBARecord *record = resultData->records[i];

        if (distinctHomeFGPercentages.find(record->homeFGPercentage) == distinctHomeFGPercentages.end()) {
            distinctHomeFGPercentages.insert(record->homeFGPercentage);
        }
    }
    for (float distinctPercentage : distinctHomeFGPercentages) {
        bPlusTree.deleteRecord(distinctPercentage);

        deleteCount++;
    }

    auto endDelete = chrono::high_resolution_clock::now();
    chrono::duration<double> time_taken_delete = endDelete - startDelete;

    BPNode* newRoot = bPlusTree.getRoot();
    bPlusTree.displayTree(newRoot);
    bPlusTree.calculateStatistics(newRoot,0);
    bPlusTree.displayRootNode();


    auto startLinearScan = chrono::high_resolution_clock::now();
    void* storagePtrDelete = storage.getStoragePtr();
    unsigned int numOfBlocks = storage.getNumOfAllocatedBlocks();
    unsigned int recordPerBlock = storage.getRecordsPerBlock();
    float lowerBound = 0.0;
    float upperBound = 0.35;

    for(unsigned int i = 0; i < numOfBlocks; i++) {
        NBARecord* recordPtr = (NBARecord*)(storagePtrDelete) + (i * recordPerBlock);
        unsigned int blockNumOfRecords = recordPerBlock;

        if(i == numOfBlocks - 1) {
            blockNumOfRecords = storage.getCurrBlockUsedSpace() / sizeof(NBARecord);
        }

        for(unsigned int j = 0; j < blockNumOfRecords; j++) {
            float fgPercentage = recordPtr->homeFGPercentage;
            if (fgPercentage >= lowerBound && fgPercentage <= upperBound) {
                bPlusTreeLS.deleteRecord(fgPercentage);
            }
            recordPtr++;
        }
    }
    auto endLinearScan = chrono::high_resolution_clock::now();
    chrono::duration<double> time_taken_linear_scan_del = endLinearScan - startLinearScan;
    cout << "The running time of the deletion process: " << time_taken_delete.count()*1000 << endl;


    cout << "The number of data blocks that would be accessed by a brute-force linear scan method: " << storage.getNumOfAllocatedBlocks() << endl;
    cout << "The running time (Linear Scan): " << time_taken_linear_scan_del.count()*1000 << endl;



return 0;
}
