#ifndef ROOM_H
#define ROOM_H

#include "AEEngine.h"
#include <vector>
#include "Utils.h" // Pulls the Rect definition from Utils.h

// Room types used for gameplay logic and visual tinting
enum class RoomType
{
    Normal,
    Start,
    Boss
};

class Room
{
public:
    // Initializes the room with geometry and default status
    Room(Rect r) : rect(r), type(RoomType::Normal), isDiscovered(false) {}

    Rect rect;              // Boundary coordinates defining the edges
    RoomType type;          // Normal, Start, or Boss
    bool isDiscovered;      // Logic for fog of war

    // Connection management for neighbor discovery logic
    void AddNeighbour(Room* neighbour) { neighbours.push_back(neighbour); }
    const std::vector<Room*>& GetNeighbours() const { return neighbours; }
    void ClearNeighbours() { neighbours.clear(); }

private:
    std::vector<Room*> neighbours; // List of rooms touching this one
};

#endif