#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include "storage.h"
#include "storage.cpp"

using namespace std;

int main() {
    Storage storage{static_cast<unsigned int>(100 * pow(10,6)), blockSize };
    ifstream inputFile("../data/games.txt");

    if (!inputFile) {
        cerr << "Failed to open the file." << endl;
        return 1;
    }

    string line;
    void *recordPtrArr[10];
    int count = 0;

    while(getline(inputFile, line) && count<10){
        count+=1;
        if(count == 1) continue;
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
        recordPtrArr[count-1] = recordPtr;
    }
    count = 1;
    while(count<10){
        auto *recordPtr = static_cast<NBARecord*>(recordPtrArr[count]);
        cout << recordPtr->date<<' '<< recordPtr->teamID << endl;
        count +=1;
    }

    inputFile.close();
    return 0;
}
