#pragma once

#ifndef ROOM_HPP
#define ROOM_HPP

#include "utils.h" 
#include <vector>

enum class RoomType { Normal, Boss, Start };

// Represents a single area in the dungeon and maintains its connectivity graph
class Room
{
public:
    // The geometric data defining the room's size and location
    Rect rect;
    RoomType type = RoomType::Normal; // Default to normal

    bool isDiscovered = false; // Tracks if the player has entered or seen this room

    // Initializes a room with a specific rectangular boundary
    Room(Rect area)
        : rect(area)
    {
    }

    // Stores a reference to an adjacent room for pathfinding or hallway logic
    void AddNeighbour(Room* neighbour)
    {
        m_neighbours.push_back(neighbour);
    }

    // Provides read-only access to the list of connected rooms
    const std::vector<Room*>& GetNeighbours() const
    {
        return m_neighbours;
    }

    // Determines if a specific room is already registered as a neighbor
    bool IsNeighbour(Room* other)
    {
        for (auto* n : m_neighbours)
        {
            if (n == other)
            {
                return true;
            }
        }
        return false;
    }

    // Calculates the shared geometric area between this room and another
    Rect Intersect(Room* other)
    {
        return rect.Intersect(other->rect);
    }

private:
    // List of non-owning pointers to adjacent rooms used to visualize connectivity
    std::vector<Room*> m_neighbours;
};

#endif