#pragma once
#ifndef ROOM_HPP
#define ROOM_HPP

#include "utils.h" 
#include <vector>
#include "Tilesets.h" 

enum class RoomType { Normal, Boss, Start };

// Represents a single area in the dungeon
// Stores the tile map for collision detection
class Room
{
public:
    // The physical boundary of the room
    Rect rect;

    // Type determines gameplay behavior like Boss or Start
    RoomType type;

    // Tracks if the player has entered this room
    bool isDiscovered;

    // Visual theme identifier for color selection
    TilesetType tilesetID;

    // The Tile Map for this specific room
    // 0 represents floor and 1 represents wall
    std::vector<std::vector<int>> tileMap;

    // Grid dimensions for the internal tile map
    int tileCountX;
    int tileCountY;
    float tileSize;

    // Constructor declaration
    Room(Rect area);

    // Destructor declaration
    ~Room();

    // Adds a connection to another room
    void AddNeighbour(Room* neighbour);

    // Returns the list of connected rooms
    const std::vector<Room*>& GetNeighbours() const;

    // Checks if a specific room is already a neighbor
    bool IsNeighbour(Room* other);

    // Clears all connections to release memory
    void ClearNeighbours();

    // Calculates the overlap with another room
    Rect Intersect(Room* other);

    // Helper to check collision
    // Return 1 for wall and 0 for floor
    int GetTile(int x, int y);

private:
    // Internal storage for connected rooms
    std::vector<Room*> m_neighbours;
};

#endif