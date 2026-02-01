#include "RoomGenerator.h"
#include "utils.h"
#include <chrono>
#include <cmath> // Included for distance calculations

// EXPLANATION: Generates a grid-based layout of rooms within the specified width and height
std::vector<std::unique_ptr<Room>> RoomGenerator::Generate(int width, int height, int roomSize)
{
    // Initialize a container to store unique pointers to Room objects for automatic memory management
    std::vector<std::unique_ptr<Room>> rooms;

    // Capture current system time to create a unique seed for the random number generator
    auto now = std::chrono::steady_clock::now();
    // Initialize the RNG with the count of ticks since the epoch started
    Random::Init(static_cast<uint32_t>(now.time_since_epoch().count()));

    // Calculate half-dimensions to center the coordinate system at (0,0)
    int halfW = width / 2;
    int halfH = height / 2;

    // Outer loop iterates from the top boundary downwards to the bottom boundary
    for (int y = halfH; y > -halfH; y -= roomSize)
    {
        // Inner loop iterates from the left boundary rightwards to the right boundary
        for (int x = -halfW; x < halfW; x += roomSize)
        {
            Rect roomRect;
            // Define the boundaries of the room: x is left, y is top, x+size is right, y-size is bottom
            roomRect.Set(x, y, x + roomSize, y - roomSize);
            // Construct a new Room on the heap and move its ownership into the vector
            rooms.push_back(std::make_unique<Room>(roomRect));
        }
    }

    // Check below for the details of this function
    FindNeighbours(rooms);

    // Ensure rooms exist before attempting to assign special room types
    if (!rooms.empty())
    {
        Room* startRoom = nullptr;

        // Initialize with a large value to ensure the first room checked becomes the initial candidate
        float minDistanceToCenter = 1000000.0f;

        // Iterate through all rooms to find which one is physically closest to the world origin (0,0)
        for (const auto& room : rooms)
        {
            AEVec2 center = room->rect.GetCenter();
            // Calculate Euclidean distance from (0,0) using the Pythagorean theorem: sqrt(a^2 + b^2)
            float dist = sqrtf(center.x * center.x + center.y * center.y);

            if (dist < minDistanceToCenter)
            {
                minDistanceToCenter = dist;
                // Store a raw pointer to the room; the unique_ptr in the vector still owns the memory
                startRoom = room.get();
            }
        }

        if (startRoom)
        {
            // Assign the Start type to the room closest to the center
            startRoom->type = RoomType::Start;

            Room* bossCandidate = nullptr;
            float maxDistance = -1.0f;
            AEVec2 startPos = startRoom->rect.GetCenter();

            // Iterate through all rooms again to find the one furthest from the Start room
            for (const auto& room : rooms)
            {
                AEVec2 currentPos = room->rect.GetCenter();
                // Calculate distance between current room and start room: sqrt((x2-x1)^2 + (y2-y1)^2)
                float dist = sqrtf(powf(currentPos.x - startPos.x, 2) + powf(currentPos.y - startPos.y, 2));
                if (dist > maxDistance)
                {
                    maxDistance = dist;
                    bossCandidate = room.get();
                }
            }

            if (bossCandidate)
            {
                // Designate the most distant room as the Boss room to maximize gameplay length
                bossCandidate->type = RoomType::Boss;
            }
        }
    }

    // Return the populated vector; ownership is transferred to the caller via move semantics
    return rooms;
}

// EXPLANATION: Links rooms that are adjacent by checking for shared borders
void RoomGenerator::FindNeighbours(std::vector<std::unique_ptr<Room>>& rooms)
{
    // A threshold of 1 is enough to detect touching walls 
    // without reaching into the next room's neighbors.
    int t = 1;

    for (size_t i = 0; i < rooms.size(); ++i)
    {
        for (size_t j = i + 1; j < rooms.size(); ++j)
        {
            Rect& a = rooms[i]->rect;
            Rect& b = rooms[j]->rect;

            // HORIZONTAL CHECK: Does Room A's Right touch Room B's Left?
            // (And do they overlap vertically?)
            bool touchRight = (a.right == b.left);
            bool touchLeft = (a.left == b.right);
            bool yOverlap = (a.top > b.bottom && a.bottom < b.top);

            // VERTICAL CHECK: Does Room A's Top touch Room B's Bottom?
            // (And do they overlap horizontally?)
            bool touchTop = (a.top == b.bottom);
            bool touchBottom = (a.bottom == b.top);
            bool xOverlap = (a.right > b.left && a.left < b.right);

            if (((touchRight || touchLeft) && yOverlap) ||
                ((touchTop || touchBottom) && xOverlap))
            {
                rooms[i]->AddNeighbour(rooms[j].get());
                rooms[j]->AddNeighbour(rooms[i].get());
            }
        }
    }
}