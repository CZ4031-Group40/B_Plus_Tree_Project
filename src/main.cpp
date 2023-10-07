#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include "BPlusTree.h"
#include "Storage.h"

using namespace std;
bool compareKey(tuple<float,void *> key1, tuple<float,void *> key2){
    return get<0>(key1) < get<0>(key2);
}

int main() {
    Storage storage{static_cast<unsigned int>(100 * pow(10,6)), blockSize };
    BPlusTree bPlusTree; // Create an empty B+ tree
    cerr << "Empty tree is initialised." << endl;
    vector<tuple<float,void *>> recordPtrs;
    vector<tuple<float,void *>> unsortedRecordPtrs;

    while(true) {
        cout << "Choose an action:" << endl;
        cout << "1. Init bulk loaded tree" << endl;
        cout << "2. Insert all record from games.txt, display at the end" << endl;
        cout << "3. Insert record to tree from test file, display after each insert" << endl;
        cout << "4. Display tree" << endl;
        cout << "5. Search key" << endl;
        cout << "6. Search key range" << endl;
        cout << "7. init new tree" << endl;
        cout << "8. delete" << endl;
        cout << "9. Exit" << endl;

        int choice;
        cin >> choice;

        switch(choice) {
            case 1: {
                ifstream inputFile("../data/test_duplicate.txt");

                if (!inputFile) {
                    cerr << "Failed to open the file." << endl;
                    return 1;
                }

                string line;
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
                    unsortedRecordPtrs.push_back(tuple<float , void*>{newRecord.homeFGPercentage, recordPtr});
                  }
                inputFile.close();

                sort(recordPtrs.begin(), recordPtrs.end(), compareKey);
                bPlusTree = BPlusTree{recordPtrs};
                break;
            }

            case 2: {
                ifstream allData("../data/test_duplicate.txt");

                if (!allData) {
                    cerr << "Failed to open the file." << endl;
                    return 1;
                }

                int numRowsToInsert;
                cout << "Enter the number of rows to insert from games.txt: ";
                cin >> numRowsToInsert;

                string line;

                int count = 0;
                getline(allData, line);
                while(count < numRowsToInsert && getline(allData, line)){
                    count+=1;
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
                        cerr << "Error reading values from the line" << count << ". Skipping the line." << endl;
                        continue; // Skip this line and move to the next one.
                    }
                    
                    void *recordPtr =  storage.storeRecord(newRecord);
                    bPlusTree.insertRecord(newRecord.homeFGPercentage,recordPtr);

                }
                allData.close();

                BPNode *ptr=bPlusTree.getRoot();
                // Display Tree
                cout << "Display Tree" << endl;
                bPlusTree.displayTree(ptr);
                cout << "Finished" << endl;
                cout << endl;
                break;
            }

            case 3: {

                ifstream sequentialInputFile("../data/test_sequential.txt");

                if (!sequentialInputFile) {
                    cerr << "Failed to open the file." << endl;
                    return 1;
                }

                string line;
                vector<tuple<float,void *>> recordPtrs;
                int count = 0;
                getline(sequentialInputFile, line);
                while(getline(sequentialInputFile, line)){
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
                    bPlusTree.insertRecord(newRecord.homeFGPercentage,recordPtr);
                    BPNode *ptr=bPlusTree.getRoot();
                    // Display Tree
                    cout << "Display Tree" << endl;
                    bPlusTree.displayTree(ptr);
                    cout << "Finished" << endl;
                    cout << endl;
                }
                sequentialInputFile.close();

                break;
            }

            case 4: {
                BPNode *ptr=bPlusTree.getRoot();

                // Display Tree
                cout << "Display Tree" << endl;
                bPlusTree.displayTree(ptr);
                cout << "Finished" << endl;
                cout << endl;
                break;
            }

            case 5: {
                // Search data
                float queriedFGP;
                cout << "Enter the FG Percentage to search: " << endl;
                cin >> queriedFGP;

                cout << "Searching for FG_PCT home = " << queriedFGP << endl;

                tuple<NBARecords *, int> queriedRecords = bPlusTree.searchRecord(queriedFGP);
                NBARecords *queriedData = get<0>(queriedRecords);

                if (queriedData == nullptr) {
                    cout << "Can't find record" << endl;
                }
                else {
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
                }

                break;
            }
            case 6: {
                float startKey;
                float endKey;

                cout << "Enter the start key to search: ";
                cin >> startKey;
                cout << "Enter the end key: ";
                cin >> endKey;

                tuple<NBARecords *, int> queriedRecords = bPlusTree.searchRangedRecord(startKey, endKey);
                NBARecords *queriedData = get<0>(queriedRecords);

                if (queriedData == nullptr) {
                    cout << "Can't find record" << endl;
                }
                else {
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
                }

                break;
            }
            case 7: {
                    bPlusTree = BPlusTree(); // Create an empty B+ tree
                    cerr << "Empty tree is initialised." << endl;
                    break;
            }
            case 8: {
                float keyToDelete;
                cout << "Enter the key to delete: ";
                cin >> keyToDelete;

                bPlusTree.deleteRecord(keyToDelete);
                break;
            }
            case 9:
                return 0;

            default:
                cout << "Invalid choice. Please choose a valid option." << endl;
        }
    }

    return 0;
}
