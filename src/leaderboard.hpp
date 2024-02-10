#include <vector>
#include <string>

std::vector<std::string> getNames();

std::vector<int> getScores();

// Checks if a score is within the top 10 scores on the leaderboard
bool checkScore(int score);

void saveScore(const char * name, int score);