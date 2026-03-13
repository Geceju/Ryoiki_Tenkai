#pragma once
#ifndef ROOM_HPP
#define ROOM_HPP

#include "utils.h" 
#include <vector>
#include "Tilesets.h" 

enum class RoomType { Normal, Boss, Start };

// represents single area in dungeon
// stores tile map for collision detection
class Room
{
public:
    // physical boundary of room
    Rect rect;

    // type determines gameplay behavior
    RoomType type;

    // tracks discovery status
    bool isDiscovered;

    // visual theme identifier
    TilesetType tilesetID;

    // tile map for specific room
    // zero represents floor and one represents wall
    std::vector<std::vector<int>> tileMap;

    // grid dimensions
    int tileCountX;
    int tileCountY;
    float tileSize;

    // constructor declaration
    Room(Rect area);

    // destructor declaration
    ~Room();

    // adds connection to another room
    void AddNeighbour(Room* neighbour);

    // returns list of adjacent rooms
    const std::vector<Room*>& GetNeighbours() const;

    // checks adjacency
    bool IsNeighbour(Room* other);

    // clears connections
    void ClearNeighbours();

    // calculates overlap
    Rect Intersect(Room* other);

    // helper to check collision
    // return one for wall and zero for floor
    int GetTile(int x, int y);

    // tracks actual maze connections
    std::vector<Room*> connectedRooms;

private:
    // internal storage for adjacent rooms
    std::vector<Room*> m_neighbours;
};

#endif