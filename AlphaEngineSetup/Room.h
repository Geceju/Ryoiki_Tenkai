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

    /**
     * @brief Registers an adjacent room as a valid connection.
     * @param neighbour Pointer to the adjoining room to be added.
     */
    void AddNeighbour(Room* neighbour);

    /**
     * @brief Retrieves all registered adjacent rooms.
     * @return A constant reference to the vector of neighboring room pointers.
     */
    const std::vector<Room*>& GetNeighbours() const;

    /**
     * @brief Checks if a specific room is already registered as a neighbor.
     * @param other Pointer to the room to verify.
     * @return True if the room is a neighbor, otherwise false.
     */
    bool IsNeighbour(Room* other);

    /**
     * @brief Removes all neighbor connections, freeing the memory array.
     */
    void ClearNeighbours();

    /**
     * @brief Calculates the overlapping geometric area between this room and another.
     * @param other Pointer to the target room.
     * @return A new Rect representing the shared space.
     */
    Rect Intersect(Room* other);

    /**
     * @brief Retrieves the collision/type value of a specific tile in the room's grid.
     * @param x The local X grid index.
     * @param y The local Y grid index.
     * @return 1 (Wall) if solid or out of bounds, 0 (Floor) if walkable.
     */
    int GetTile(int x, int y);

    // tracks actual maze connections
    std::vector<Room*> connectedRooms;

private:
    // internal storage for adjacent rooms
    std::vector<Room*> m_neighbours;
};

#endif