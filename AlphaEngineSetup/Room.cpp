#include "Room.h"

// Constructor
Room::Room(Rect area) : rect(area)
{
    type = RoomType::Normal;
    isDiscovered = false;

    // Default to the first style in your list
    tilesetID = TilesetType::Type_01;
}

// Destructor
Room::~Room()
{
    ClearNeighbours();
}

void Room::AddNeighbour(Room* neighbour)
{
    // Only add if the pointer is valid and not already in our list
    if (neighbour != nullptr && !IsNeighbour(neighbour))
    {
        m_neighbours.push_back(neighbour);
    }
}

const std::vector<Room*>& Room::GetNeighbours() const
{
    return m_neighbours;
}

bool Room::IsNeighbour(Room* other)
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

void Room::ClearNeighbours()
{
    m_neighbours.clear();
    m_neighbours.shrink_to_fit();
}

Rect Room::Intersect(Room* other)
{
    return rect.Intersect(other->rect);
}