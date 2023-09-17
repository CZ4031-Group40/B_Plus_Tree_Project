#ifndef B_PLUS_TREE_PROJECT_DATASTRUCT_H
#define B_PLUS_TREE_PROJECT_DATASTRUCT_H

struct NBARecord{
    unsigned int teamID; //4 bytes
    unsigned int homePoints; //4 bytes
    float homeFGPercentage; //4 bytes
    float homeFTPercentage; //4 bytes
    float homeFG3Percentage; //4 bytes
    unsigned int homeAssist; //4 bytes
    unsigned int homeRebound; //4 bytes
    char date[11];
    bool homeTeamWins; //1 byte
};

#endif
