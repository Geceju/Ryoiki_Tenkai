#pragma once
#include <vector>
#include <string>

struct RunRecord {
    std::string playerName;
    int levelReached;
    float timeTaken;
};

class LeaderboardSystem {
public:
    static void Load();
    static void Save();
    static void AddRun(const std::string& name, int level, float time);
    static const std::vector<RunRecord>& GetTopRuns();

private:
    static std::vector<RunRecord> s_TopRuns;
};

// Global Tracker Variables for the active game
extern float g_RunTimer;
extern int g_CurrentRunLevel;