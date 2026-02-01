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
    // Explicit destructor to ensure the neighbors vector is released
    ~Room()
    {
        ClearNeighbours();
    }

    // The geometric boundary defining the room's physical footprint; 
    // Used for collision, discovery logic, and as the source for mesh scaling
    Rect rect;

    // Default to normal
    RoomType type = RoomType::Normal; 

    // Tracks if the player has entered or seen this room
    bool isDiscovered = false; 

    // Initializes a room with a specific rectangular boundary
    Room(Rect area) : rect(area) {}

    // Stores a reference to an adjacent room for pathfinding or hallway logic
    // Includes a safety check to prevent duplicate neighbor entries
    void AddNeighbour(Room* neighbour)
    {
        // Only add if the pointer is valid and not already in our list
        if (neighbour != nullptr && !IsNeighbour(neighbour))
        {
            m_neighbours.push_back(neighbour);
        }
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

    // Clears the neighbor list to break memory cycles during cleanup
    void ClearNeighbours()
    {
        m_neighbours.clear();
        m_neighbours.shrink_to_fit();
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