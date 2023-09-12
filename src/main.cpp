#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

int main() {
    ifstream inputFile("../data/games.txt");

    if (!inputFile) {
        std::cerr << "Failed to open the file." << std::endl;
        return 1;
    }

    string line;
    int count = 0;

    while(getline(inputFile, line) && count<10){
        count+=1;
        istringstream iss(line);
        string token;

        while (std::getline(iss, token, ' ')) {
            // Process each token (space-separated value)
            std::cout << "Token: " << token << std::endl;
        }
    }

    inputFile.close();
    return 0;
}
