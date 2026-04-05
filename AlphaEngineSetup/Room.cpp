//author : Tay Dylan
#include "Room.h"

// Constructor initializes defaults
Room::Room(Rect area) : rect(area)
{
    // Default room type is Normal
    type = RoomType::Normal;

    // Room starts hidden
    isDiscovered = false;

    // Default visual style
    tilesetID = TilesetType::Type_01;

    // Initialize defaults to zero
    tileCountX = 0;
    tileCountY = 0;
    tileSize = 0.0f;
}

// Destructor cleans up neighbor references
Room::~Room()
{
    ClearNeighbours();
}

// Adds a neighbor if not already present
void Room::AddNeighbour(Room* neighbour)
{
    // Ensure pointer is valid and not a duplicate
    if (neighbour != nullptr && !IsNeighbour(neighbour))
    {
        m_neighbours.push_back(neighbour);
    }
}

// Accessor for neighbor list
const std::vector<Room*>& Room::GetNeighbours() const
{
    return m_neighbours;
}

// Checks if a room is already in the neighbor list
bool Room::IsNeighbour(Room* other)
{
    // Iterate through existing neighbors
    for (auto* n : m_neighbours)
    {
        // Check memory address equality
        if (n == other)
        {
            return true;
        }
    }
    return false;
}

// Clears the list and frees vector memory
void Room::ClearNeighbours()
{
    m_neighbours.clear();
    m_neighbours.shrink_to_fit();
}

// Wrapper for rectangle intersection
Rect Room::Intersect(Room* other)
{
    return rect.Intersect(other->rect);
}

// Check the grid value
// Return 1 meaning Wall if out of bounds
int Room::GetTile(int x, int y)
{
    // Safety check for uninitialized map
    if (tileMap.empty())
    {
        return 1;
    }

    // Boundary check to prevent access violation
    if (y < 0 || y >= tileCountY || x < 0 || x >= tileCountX)
    {
        return 1;
    }

    // Return the specific tile value
    return tileMap[y][x];
}