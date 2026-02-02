#include "Room.h"

Room::Room(Rect area) : rect(area)
{
    type = RoomType::Normal;
    isDiscovered = false;

    // [FIX] Using Type_01 as requested
    tilesetID = TilesetType::Type_01;
}

Room::~Room()
{
    ClearNeighbours();
}

void Room::AddNeighbour(Room* neighbour)
{
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
        if (n == other) return true;
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