#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
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
    unsigned int recordsPerBlock = storage.getRecordsPerBlock();
    cout << "Number of records per block (theoretical): " << recordsPerBlock << endl;

    cout << "========================================" << endl;
    unsigned int allocatedBlocks = storage.getNumOfAllocatedBlocks();
    cout << "Current number of blocks used to store the data: " << allocatedBlocks << endl;

    cout << "========================================" << endl;
    unsigned int empiricalRecordsPerBlock = numOfRecords / allocatedBlocks;
    cout << "Number of records per block (empirical): " << empiricalRecordsPerBlock << endl;

    // Print row numbers with errors at the end
    if (!errorRows.empty()) {
        cout << "Rows with errors:";
        for (int row : errorRows) {
            cout << " " << row << ",";
        }
        cout << endl;
    }

    cout << "========================================EXPERIMENT TWO========================================" << endl;

    BPlusTree bPlusTree; // Create an empty B+ tree

    for (auto it = recordPtrs.end() - 1; it >= recordPtrs.begin(); --it) {
        float homeFGPercentage = get<0>(*it);
        void* recordPtr = get<1>(*it);

        // Insert the record into the bPlusTree
        bPlusTree.insertRecord(homeFGPercentage, recordPtr);
    }

    return 0;
}
