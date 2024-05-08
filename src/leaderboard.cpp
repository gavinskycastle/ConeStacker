#include <filesystem>

#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
#include <string>
#include <algorithm>

#include "main.hpp"
#include <iostream>
 
std::ofstream oScoreFile;
std::ifstream iScoreFile;

// Set save location to /save/leaderboard instead if on web
#if defined(PLATFORM_WEB)
    std::string scoreFileLocation = "save/leaderboard";
#else
    std::string scoreFileLocation = "../save/leaderboard";
#endif

void selectLeaderboardMode(GameState mode) {
    #if defined(PLATFORM_WEB)
        scoreFileLocation = "save/leaderboard";
    #else
        scoreFileLocation = "../save/leaderboard";
    #endif
    std::string scoreFileLocation_S = scoreFileLocation;
    switch (mode) {
        case PLAY_CLASSIC:
            scoreFileLocation_S += "_classic";
            break;
        case PLAY_ARCADE:
            scoreFileLocation_S += "_arcade";
            break;
        case PLAY_RHYTHM:
            scoreFileLocation_S += "_rhythm";
            break;
        case PLAY_DUELS:
            scoreFileLocation_S += "_duels";
            break;
        default:
            break;
    }
    scoreFileLocation = scoreFileLocation_S;
}

void open_oScoreFile() {
    #if defined(PLATFORM_WEB)
        std::filesystem::create_directory("save");
    #else
        std::filesystem::create_directory("../save");
    #endif
    
    oScoreFile = std::ofstream(scoreFileLocation.c_str());
}

void close_oScoreFile() {
    if (oScoreFile.is_open()) {
        oScoreFile.close();
    }
}

void open_iScoreFile() {
    iScoreFile = std::ifstream(scoreFileLocation.c_str());
}

void close_iScoreFile() {
    if (iScoreFile.is_open()) {
        iScoreFile.close();
    }
}

std::vector<std::string> getScoreFileEntries() {
    open_iScoreFile();
    
    std::vector<std::string> fileEntries;
    
    std::string entry;
    while (std::getline(iScoreFile, entry)) {
        fileEntries.push_back(entry);
    }
    
    close_iScoreFile();
    
    return fileEntries;
}

std::vector<std::string> getScoreFileColumn(int column) {
    std::vector<std::string> fileEntries = getScoreFileEntries();
    
    std::vector<std::string> columnEntries;
    
    for (std::string entry : fileEntries) {
        std::vector<std::string> rowEntries;
        std::stringstream ss (entry);
        std::string item;

        while (getline(ss, item, ',')) {
            rowEntries.push_back(item);
        }
        
        columnEntries.push_back(rowEntries[column]);
    }
    
    return columnEntries;
}

bool isScoreFileEmpty() {
    bool isEmpty;
    
    open_iScoreFile();
    iScoreFile.seekg(0, std::ios::end);  
    isEmpty = (iScoreFile.tellg() == 0);
    
    close_iScoreFile();
    
    return isEmpty;
}

std::vector<std::string> getNames() {
    return getScoreFileColumn(0);
}

std::vector<int> getScores() {
    std::vector<int> scores;
    
    std::vector<std::string> scoreStrings = getScoreFileColumn(1);
    
    for (std::string scoreString : scoreStrings) {
        scores.push_back(std::stoi(scoreString));
    }
    
    return scores;
}

std::vector<int> getTimestamps() {
    std::vector<int> scores;
    
    std::vector<std::string> scoreStrings = getScoreFileColumn(2);
    
    for (std::string scoreString : scoreStrings) {
        scores.push_back(std::stoi(scoreString));
    }
    
    return scores;
}

// Returns where a score would be placed if placed in the leaderboard
int scorePlacement(int score) {
    std::vector<int> scores = getScores();
    
    int index = 0;
    
    for (long unsigned int i = 0; i < scores.size(); ++i) {
        if (scores[i] > score) {
            index = i + 1;
        }
    }
    
    return index;
}

// Checks if a score is within the top 10 scores on the leaderboard
bool checkScore(int score) {
    return scorePlacement(score) < 10;
}

void saveScore(const char * name, int score) {
    // Remove commas from name
    std::string nameString = name;
    nameString.erase(std::remove(nameString.begin(), nameString.end(), ','), nameString.end());
    name = nameString.c_str();
    
    std::stringstream sScoreFileEntry;
    
    sScoreFileEntry << name << "," << std::to_string(score) << "," << std::to_string(std::time(0));
    
    std::string scoreFileEntry = sScoreFileEntry.str();
    
    if (!isScoreFileEmpty()) {
        std::vector<std::string> scoreFileEntries = getScoreFileEntries();
        int placement = scorePlacement(score);
        
        scoreFileEntries.insert(scoreFileEntries.begin() + placement, scoreFileEntry);
        
        open_oScoreFile();
        oScoreFile << ""; // Clears file
        
        for (std::string entry : scoreFileEntries) {
            oScoreFile << entry << std::endl;
        }
        
        close_oScoreFile();
    } else {
        open_oScoreFile();
        oScoreFile << ""; // Clears file
        
        oScoreFile << scoreFileEntry;
        
        close_oScoreFile();
    }
}
