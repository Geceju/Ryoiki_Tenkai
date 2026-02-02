// [FIX 1] STANDARD INCLUDES MUST COME FIRST
#include <chrono>
#include <cmath>
#include <algorithm>
#include <vector>
#include <memory>

// [FIX 2] PROJECT INCLUDES COME SECOND
#include "RoomGenerator.h"
#include "Utils.h"
#include "Tilesets.h" // Needed for TilesetManager

// EXPLANATION: Generates a grid-based layout of rooms within the specified width and height
std::vector<std::unique_ptr<Room>> RoomGenerator::Generate(int width, int height, int roomSize)
{
    // [FIX 3] CRITICAL: Load the tileset data before generating to prevent crashes
    TilesetManager::Load();

    // Initialize a container to store unique pointers to Room objects
    std::vector<std::unique_ptr<Room>> rooms;

    // Capture current system time to create a unique seed
    auto now = std::chrono::steady_clock::now();

    // Initialize the RNG
    Random::Init(static_cast<uint32_t>(now.time_since_epoch().count()));

    // Calculate half-dimensions to center the coordinate system at (0,0)
    int halfW = width / 2;
    int halfH = height / 2;

    // Outer loop iterates from the top boundary downwards
    for (int y = halfH; y > -halfH; y -= roomSize)
    {
        // Inner loop iterates from the left boundary rightwards
        for (int x = -halfW; x < halfW; x += roomSize)
        {
            Rect roomRect;
            roomRect.Set(x, y, x + roomSize, y - roomSize);

            auto newRoom = std::make_unique<Room>(roomRect);

            // [FIX 4] Assign a random visual style to the room
            newRoom->tilesetID = TilesetManager::GetRandom();

            rooms.push_back(std::move(newRoom));
        }
    }

    FindNeighbours(rooms);

    // Ensure rooms exist before attempting to assign special room types
    if (!rooms.empty())
    {
        Room* startRoom = nullptr;
        float minDistanceToCenter = 1000000.0f;

        // Find Start Room (Closest to 0,0)
        for (const auto& room : rooms)
        {
            AEVec2 center = room->rect.GetCenter();
            float dist = sqrtf(center.x * center.x + center.y * center.y);

            if (dist < minDistanceToCenter)
            {
                minDistanceToCenter = dist;
                startRoom = room.get();
            }
        }

        if (startRoom)
        {
            startRoom->type = RoomType::Start;

            // Find Boss Room (Furthest from Start)
            Room* bossCandidate = nullptr;
            float maxDistance = -1.0f;
            AEVec2 startPos = startRoom->rect.GetCenter();

            for (const auto& room : rooms)
            {
                AEVec2 currentPos = room->rect.GetCenter();
                float dist = sqrtf(powf(currentPos.x - startPos.x, 2) + powf(currentPos.y - startPos.y, 2));
                if (dist > maxDistance)
                {
                    maxDistance = dist;
                    bossCandidate = room.get();
                }
            }

            if (bossCandidate)
            {
                bossCandidate->type = RoomType::Boss;
            }
        }
    }

    return rooms;
}

// EXPLANATION: Links rooms that are adjacent by checking for shared borders
void RoomGenerator::FindNeighbours(std::vector<std::unique_ptr<Room>>& rooms)
{
    int t = 1;

    for (size_t i = 0; i < rooms.size(); ++i)
    {
        for (size_t j = i + 1; j < rooms.size(); ++j)
        {
            Rect& a = rooms[i]->rect;
            Rect& b = rooms[j]->rect;

            bool touchRight = (a.right == b.left);
            bool touchLeft = (a.left == b.right);
            bool yOverlap = (a.top > b.bottom && a.bottom < b.top);

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