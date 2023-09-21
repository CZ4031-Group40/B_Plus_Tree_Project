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

    // Display Tree
    cout << "Display Tree" << endl;
    bPlusTree.displayTree(ptr);
    cout << "Finished" << endl;
    cout << endl;

    // Search data
    float queriedFGP = 0.5;
    NBARecords *queriedData = bPlusTree.searchRecord(queriedFGP);
    cout << "Searching for FG_PCT home = " << queriedFGP << endl;
    cout << "GAME_DATE_EST  TEAM_ID_home    PTS_home    FG_PCT_home     FT_PCT_home     FG3_PCT_home    AST_home    REB_home	    HOME_TEAM_WINS" << endl;
    for (int i = 0; i < queriedData->records.size(); i++) {
        NBARecord *record = queriedData->records[i];
        cout << record->date << "\t\t";
        cout << record->teamID << "\t\t";
        cout << record->homePoints << "\t\t\t";
        cout << fixed << setprecision(3) << record->homeFGPercentage << "\t\t\t";
        cout << fixed << setprecision(3) << record->homeFTPercentage << "\t\t\t";
        cout << fixed << setprecision(3) << record->homeFG3Percentage << "\t\t\t";
        cout << record->homeAssist << "\t\t\t";
        cout << record->homeRebound << "\t\t\t";
        cout << record->homeTeamWins << endl;
    }

    cout<<ptr->getKeys()[0]<<endl;
    return 0;
}
