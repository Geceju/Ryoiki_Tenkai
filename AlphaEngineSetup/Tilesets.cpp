#include "Tilesets.h"
#include <cstdlib> // For rand()

// Define the static list
std::vector<TilesetData> TilesetManager::s_List;

void TilesetManager::Load()
{
    // Safety: Only load if list is empty
    if (!s_List.empty()) return;

    s_List.resize((int)TilesetType::COUNT);

    // --- DEFINE COLORS HERE ---

    // Type 01: Standard Grey
    s_List[(int)TilesetType::Type_01] = { 0.5f, 0.5f, 0.5f };

    // Type 02: Bright Red
    s_List[(int)TilesetType::Type_02] = { 0.8f, 0.2f, 0.2f };

    // Type 03: Bright Blue
    s_List[(int)TilesetType::Type_03] = { 0.2f, 0.6f, 1.0f };

    // Type 04: Bright Green
    s_List[(int)TilesetType::Type_04] = { 0.2f, 0.8f, 0.2f };
}

const TilesetData& TilesetManager::Get(TilesetType type)
{
    // Safety: If list is empty (Load wasn't called), return white dummy
    if (s_List.empty()) {
        static TilesetData def = { 1.0f, 1.0f, 1.0f };
        return def;
    }

    int i = (int)type;
    // Safety: If index is invalid, return Type_01
    if (i < 0 || i >= s_List.size()) return s_List[0];

    return s_List[i];
}

TilesetType TilesetManager::GetRandom()
{
    // Pick a random number between 0 and COUNT-1
    return (TilesetType)(rand() % (int)TilesetType::COUNT);
}