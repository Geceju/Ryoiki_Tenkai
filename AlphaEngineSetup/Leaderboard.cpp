#include "Leaderboard.h"
#include <fstream>
#include <algorithm>

std::vector<RunRecord> LeaderboardSystem::s_TopRuns;

float g_RunTimer = 0.0f;
int g_CurrentRunLevel = 1;

// Sorting rule: Highest level first. If tied, lowest time first.
bool CompareRuns(const RunRecord& a, const RunRecord& b) {
    if (a.levelReached != b.levelReached)
        return a.levelReached > b.levelReached;
    return a.timeTaken < b.timeTaken;
}

void LeaderboardSystem::Load() {
    s_TopRuns.clear();
    std::ifstream file("leaderboard.txt");
    if (file.is_open()) {
        std::string name;
        int lvl;
        float time;
        // Read all 3 variables!
        while (file >> name >> lvl >> time) {
            s_TopRuns.push_back({ name, lvl, time });
        }
        file.close();
    }
    std::sort(s_TopRuns.begin(), s_TopRuns.end(), CompareRuns);
}

void LeaderboardSystem::Save() {
    std::ofstream file("leaderboard.txt");
    if (file.is_open()) {
        for (const auto& run : s_TopRuns) {
            // Write the name first
            std::string n = run.playerName.empty() ? "UNKNOWN" : run.playerName;
            file << n << " " << run.levelReached << " " << run.timeTaken << "\n";
        }
        file.close();
    }
}

void LeaderboardSystem::AddRun(const std::string& name, int level, float time) {
    s_TopRuns.push_back({ name, level, time });
    std::sort(s_TopRuns.begin(), s_TopRuns.end(), CompareRuns);

    if (s_TopRuns.size() > 10) {
        s_TopRuns.resize(10);
    }
    Save();
}

const std::vector<RunRecord>& LeaderboardSystem::GetTopRuns() {
    return s_TopRuns;
}