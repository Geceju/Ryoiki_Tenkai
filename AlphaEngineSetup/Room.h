#pragma once
#ifndef ROOM_HPP
#define ROOM_HPP

#include "utils.h" 
#include <vector>
#include "Tilesets.h" 

enum class RoomType { Normal, Boss, Start };

class Room
{
public:
    // Data
    Rect rect;
    RoomType type;
    bool isDiscovered;
    TilesetType tilesetID;

    // --- DECLARATIONS ONLY (No Code Bodies!) ---
    Room(Rect area);
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