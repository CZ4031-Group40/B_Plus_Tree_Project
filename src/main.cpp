#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "BPlusTree.h"
#include "Storage.h"

using namespace std;
bool compareKey(tuple<float,void *> key1, tuple<float,void *> key2){
    return get<0>(key1) < get<0>(key2);
}

int main() {
    Storage storage{static_cast<unsigned int>(100 * pow(10,6)), blockSize };
    ifstream inputFile("../data/test_duplicate.txt");

    if (!inputFile) {
        cerr << "Failed to open the file." << endl;
        return 1;
    }

    string line;
    vector<tuple<float,void *>> recordPtrs;
    int count = 0;
    getline(inputFile, line);
    while(getline(inputFile, line)){
        count+=1;
        istringstream iss(line);
        NBARecord newRecord{};
        iss >> newRecord.date;
        iss >> newRecord.teamID;
        iss >> newRecord.homePoints;
        iss >> newRecord.homeFGPercentage;
        iss >> newRecord.homeFTPercentage;
        iss >> newRecord.homeFG3Percentage;
        iss >> newRecord.homeAssist;
        iss >> newRecord.homeRebound;
        iss >> newRecord.homeTeamWins;

        void *recordPtr =  storage.storeRecord(newRecord);
        recordPtrs.push_back(tuple<float , void*>{newRecord.homeFGPercentage, recordPtr});
    }
    inputFile.close();

    sort(recordPtrs.begin(), recordPtrs.end(), compareKey);

    BPlusTree bPlusTree{recordPtrs};

    BPNode *ptr=bPlusTree.getRoot();
    cout<<ptr->getKeys()[0]<<endl;
    return 0;
}
