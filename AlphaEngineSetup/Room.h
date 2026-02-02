#pragma once
#ifndef ROOM_HPP
#define ROOM_HPP

#include "utils.h" 
#include <vector>
// [IMPORTANT] Include this so the Room knows what a "TilesetType" is
#include "Tilesets.h" 

enum class RoomType { Normal, Boss, Start };

// Represents a single area in the dungeon
class Room
{
public:
    // --- Data Members ---
    Rect rect;
    RoomType type;
    bool isDiscovered;

    // [New System] The ID of the visual style assigned to this room
    TilesetType tilesetID;

    // --- Function Declarations (NO BODIES) ---

    // Constructor
    Room(Rect area);

    // Destructor
    ~Room();

    void AddNeighbour(Room* neighbour);
    const std::vector<Room*>& GetNeighbours() const;
    bool IsNeighbour(Room* other);
    void ClearNeighbours();
    Rect Intersect(Room* other);

private:
    std::vector<Room*> m_neighbours;
};

#endif