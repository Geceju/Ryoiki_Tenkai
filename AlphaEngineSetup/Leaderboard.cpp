//author : Tay Dylan
#include "Leaderboard.h"
#include <fstream>
#include <algorithm>

// Static storage for the top run records
std::vector<RunRecord> LeaderboardSystem::s_TopRuns;

// Tracks the time elapsed during the current gameplay session
float g_RunTimer = 0.0f;

// Tracks the current dungeon floor the player is on
int g_CurrentRunLevel = 1;

// Sorting rule:
// Prioritizes higher levels first, then faster times for tie-breaking
bool CompareRuns(const RunRecord& a, const RunRecord& b)
{
    if (a.levelReached != b.levelReached) return a.levelReached > b.levelReached;

    return a.timeTaken < b.timeTaken;
}

// Reads the leaderboard data from the local text file
void LeaderboardSystem::Load()
{
    // Reset the current list before loading new data
    s_TopRuns.clear();

    std::ifstream file("leaderboard.txt");

    // Only proceed if the file exists and is accessible
    if (file.is_open())
    {
        std::string name;
        int lvl;
        float time;

        // Extract formatted data sequentially until end of file
        // Note: This expects names without spaces
        while (file >> name >> lvl >> time) s_TopRuns.push_back({ name, lvl, time });

        file.close();
    }
    // Ensure the loaded list adheres to ranking rules
    std::sort(s_TopRuns.begin(), s_TopRuns.end(), CompareRuns);
}

// Save the current top runs to a local text file
void LeaderboardSystem::Save()
{
    // Opening with ofstream creates the file if it is missing
    std::ofstream file("leaderboard.txt");

    if (file.is_open())
    {
        for (const auto& run : s_TopRuns)
        {
            // Provide a fallback string if the player name is null
            std::string n = run.playerName.empty() ? "UNKNOWN" : run.playerName;

            // Space-separated format for easy extraction
            file << n << " " << run.levelReached << " " << run.timeTaken << "\n";
        }
        file.close();
    }
}

// Submits a new score to the system and updates the persistent storage
void LeaderboardSystem::AddRun(const std::string& name, int level, float time)
{
    // Append the new record to the existing list
    s_TopRuns.push_back({ name, level, time });

    // Re-rank the list with the new entry included
    std::sort(s_TopRuns.begin(), s_TopRuns.end(), CompareRuns);

    // Maintain a maximum limit of 10 entries for the leaderboard
    if (s_TopRuns.size() > 10) s_TopRuns.resize(10);

    // Update the physical file to reflect the new rankings
    Save();
}

// Provides read-only access to the current leaderboard list
const std::vector<RunRecord>& LeaderboardSystem::GetTopRuns()
{
    return s_TopRuns;
}