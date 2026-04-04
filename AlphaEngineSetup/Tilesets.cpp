#include "Tilesets.h"
#include <random>

std::vector<TilesetData> TilesetManager::s_List;

void TilesetManager::Load() {
    s_List.clear();
    s_List.resize((size_t)TilesetType::Count);

    // Define unique colors for each tileset type 
    s_List[(int)TilesetType::Type_01] = { 0.4f, 0.4f, 0.4f, nullptr }; // Gray
    s_List[(int)TilesetType::Type_02] = { 0.2f, 0.2f, 0.5f, nullptr }; // Blue
    s_List[(int)TilesetType::Type_03] = { 0.2f, 0.5f, 0.2f, nullptr }; // Green
    s_List[(int)TilesetType::Type_04] = { 0.5f, 0.2f, 0.2f, nullptr }; // Red
    s_List[(int)TilesetType::Type_05] = { 0.4f, 0.2f, 0.5f, nullptr }; // Purple
}

void TilesetManager::Unload() {
    s_List.clear();
}

const TilesetData& TilesetManager::Get(TilesetType type) {
    return s_List[(int)type];
}

TilesetType TilesetManager::GetRandom() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, (int)TilesetType::Count - 1);
    return (TilesetType)dis(gen);
}