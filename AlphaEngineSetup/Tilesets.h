#pragma once
#ifndef TILESETS_H
#define TILESETS_H

#include "AEEngine.h"
#include <vector>

// The menu of styles (Numeric/Generic)
enum class TilesetType {
    Type_01,      // Grey (Standard)
    Type_02,      // Red
    Type_03,      // Blue
    Type_04,      // Green
    COUNT         // Helper to count how many we have
};

// The Data for a single style (Color)
struct TilesetData {
    float r, g, b; // Red, Green, Blue (0.0 to 1.0)
};

// 3. The Manager
class TilesetManager {
public:
    static void Load(); // Call this once to build the list
    static const TilesetData& Get(TilesetType type);
    static TilesetType GetRandom();

private:
    static std::vector<TilesetData> s_List;
};

#endif