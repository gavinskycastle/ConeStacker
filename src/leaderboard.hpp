#include <fstream>
#include <vector>
#include <string>

void open_oScoreFile();
void close_oScoreFile();

void open_iScoreFile();
void close_iScoreFile();

std::vector<std::string> getScoreFileEntries();

std::vector<std::string> getScoreFileColumn(int column);

bool isScoreFileEmpty();

std::vector<std::string> getNames();

std::vector<int> getScores();

std::vector<int> getTimestamps();

// Returns where a score would be placed if placed in the leaderboard
int scorePlacement(int score);

// Checks if a score is within the top 10 scores on the leaderboard
bool checkScore(int score);

void saveScore(const char * name, int score);