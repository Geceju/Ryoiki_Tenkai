#include <chrono>
#include <cmath>
#include <algorithm>
#include <vector>
#include <memory>
#include <stack>
#include <set>

#include "RoomGenerator.h"
#include "utils.h"
#include "Tilesets.h"

// Helper function to carve a connection between two adjacent rooms
void CarveDoor(Room* roomA, Room* roomB)
{
    // Determine direction using centers
    AEVec2 centerA = roomA->rect.GetCenter();
    AEVec2 centerB = roomB->rect.GetCenter();

    int w = roomA->tileCountX;
    int h = roomA->tileCountY;
    int midX = w / 2;
    int midY = h / 2;

    // A is Below B (B is Top Neighbor)
    if (centerB.y > centerA.y)
    {
        // Open the door on A's Top Edge
        roomA->tileMap[0][midX] = 0;
        roomA->tileMap[0][midX - 1] = 0;

        // SAFEGUARD: Clear the internal tiles directly below the door
        // This ensures no random obstacle blocks entry
        roomA->tileMap[1][midX] = 0;
        roomA->tileMap[1][midX - 1] = 0;

        // Open the door on B's Bottom Edge
        roomB->tileMap[h - 1][midX] = 0;
        roomB->tileMap[h - 1][midX - 1] = 0;

        // SAFEGUARD: Clear internal tiles directly above the door
        roomB->tileMap[h - 2][midX] = 0;
        roomB->tileMap[h - 2][midX - 1] = 0;
    }
    // A is Above B (B is Bottom Neighbor)
    else if (centerB.y < centerA.y)
    {
        // Open A's Bottom Edge
        roomA->tileMap[h - 1][midX] = 0;
        roomA->tileMap[h - 1][midX - 1] = 0;

        // SAFEGUARD
        roomA->tileMap[h - 2][midX] = 0;
        roomA->tileMap[h - 2][midX - 1] = 0;

        // Open B's Top Edge
        roomB->tileMap[0][midX] = 0;
        roomB->tileMap[0][midX - 1] = 0;

        // SAFEGUARD
        roomB->tileMap[1][midX] = 0;
        roomB->tileMap[1][midX - 1] = 0;
    }
    // A is Left of B (B is Right Neighbor)
    else if (centerB.x > centerA.x)
    {
        // Open A's Right Edge
        roomA->tileMap[midY][w - 1] = 0;
        roomA->tileMap[midY - 1][w - 1] = 0;

        // SAFEGUARD
        roomA->tileMap[midY][w - 2] = 0;
        roomA->tileMap[midY - 1][w - 2] = 0;

        // Open B's Left Edge
        roomB->tileMap[midY][0] = 0;
        roomB->tileMap[midY - 1][0] = 0;

        // SAFEGUARD
        roomB->tileMap[midY][1] = 0;
        roomB->tileMap[midY - 1][1] = 0;
    }
    // A is Right of B (B is Left Neighbor)
    else if (centerB.x < centerA.x)
    {
        // Open A's Left Edge
        roomA->tileMap[midY][0] = 0;
        roomA->tileMap[midY - 1][0] = 0;

        // SAFEGUARD
        roomA->tileMap[midY][1] = 0;
        roomA->tileMap[midY - 1][1] = 0;

        // Open B's Right Edge
        roomB->tileMap[midY][w - 1] = 0;
        roomB->tileMap[midY - 1][w - 1] = 0;

        // SAFEGUARD
        roomB->tileMap[midY][w - 2] = 0;
        roomB->tileMap[midY - 1][w - 2] = 0;
    }
}

std::vector<std::unique_ptr<Room>> RoomGenerator::Generate(int width, int height, int roomSize)
{
    TilesetManager::Load();

    std::vector<std::unique_ptr<Room>> rooms;

    auto now = std::chrono::steady_clock::now();
    Random::Init(static_cast<uint32_t>(now.time_since_epoch().count()));

    int halfW = width / 2;
    int halfH = height / 2;

    // Create Grid of Rooms
    for (int y = halfH; y > -halfH; y -= roomSize)
    {
        for (int x = -halfW; x < halfW; x += roomSize)
        {
            Rect roomRect;
            roomRect.Set(x, y, x + roomSize, y - roomSize);

            auto newRoom = std::make_unique<Room>(roomRect);
            newRoom->tilesetID = TilesetManager::GetRandom();

            newRoom->tileCountX = 16;
            newRoom->tileCountY = 16;
            newRoom->tileSize = (float)roomSize / 16.0f;

            // Initialize all rooms as solid blocks first
            // We will carve doors strictly based on the maze algorithm
            newRoom->tileMap.resize(16, std::vector<int>(16, 1));

            rooms.push_back(std::move(newRoom));
        }
    }

    // Link Neighbors logically
    // This builds the graph edges we can potentially travel
    FindNeighbours(rooms);

    // Carve Internal Random Obstacles
    // This makes the room look like a dungeon not a solid block
    for (auto& room : rooms)
    {
        // Keep outer edge as walls index 0 and 15
        for (int y = 1; y < 15; ++y)
        {
            for (int x = 1; x < 15; ++x)
            {
                if (Random::Range(0, 100) > 10)
                    room->tileMap[y][x] = 0; // Floor
                else
                    room->tileMap[y][x] = 1; // Obstacle
            }
        }
    }

    // MAZE GENERATION (Recursive Backtracker)
    // This ensures a guaranteed path from Start to Boss
    // It also creates the 0, 1, 2, 3 exit variations naturally

    if (!rooms.empty())
    {
        // Find Start Room (Closest to center)
        Room* startRoom = nullptr;
        float minDist = 1000000.0f;
        for (const auto& room : rooms)
        {
            AEVec2 c = room->rect.GetCenter();
            float d = sqrtf(c.x * c.x + c.y * c.y);
            if (d < minDist) { minDist = d; startRoom = room.get(); }
        }

        if (startRoom)
        {
            std::stack<Room*> stack;
            std::set<Room*> visited;

            stack.push(startRoom);
            visited.insert(startRoom);

            while (!stack.empty())
            {
                Room* current = stack.top();
                std::vector<Room*> unvisitedNeighbors;

                // Find all adjacent rooms we have not visited yet
                for (auto* neighbor : current->GetNeighbours())
                {
                    if (visited.find(neighbor) == visited.end())
                    {
                        unvisitedNeighbors.push_back(neighbor);
                    }
                }

                if (!unvisitedNeighbors.empty())
                {
                    // Choose random neighbor
                    int idx = Random::Range(0, (int)unvisitedNeighbors.size() - 1);
                    Room* next = unvisitedNeighbors[idx];

                    // Carve a door between current and next
                    // This now guarantees the path is clear of obstacles
                    CarveDoor(current, next);

                    // Mark visited and push to stack
                    visited.insert(next);
                    stack.push(next);
                }
                else
                {
                    // Dead end backtrack
                    stack.pop();
                }
            }

            // Optional
            // Randomly add a few extra doors to create loops
            // Prevents the dungeon from being a perfect tree
            for (auto& room : rooms)
            {
                if (Random::Range(0, 100) < 10) // 10 percent chance
                {
                    auto& neighbors = room->GetNeighbours();
                    if (!neighbors.empty())
                    {
                        Room* randomNeighbor = neighbors[Random::Range(0, (int)neighbors.size() - 1)];
                        CarveDoor(room.get(), randomNeighbor);
                    }
                }
            }
        }
    }

    // Assign Start and Boss Types
    if (!rooms.empty())
    {
        Room* startRoom = nullptr;
        float minDistanceToCenter = 1000000.0f;

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
            startRoom->tilesetID = TilesetType::Type_01;

            // Clear start room obstacles completely
            for (int y = 1; y < 15; ++y)
                for (int x = 1; x < 15; ++x)
                    startRoom->tileMap[y][x] = 0;

            // Find Boss Room furthest from Start
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

void RoomGenerator::FindNeighbours(std::vector<std::unique_ptr<Room>>& rooms)
{
    // O(N^2) but acceptable for < 1000 rooms
    for (size_t i = 0; i < rooms.size(); ++i)
    {
        for (size_t j = i + 1; j < rooms.size(); ++j)
        {
            Rect& a = rooms[i]->rect;
            Rect& b = rooms[j]->rect;

            // Check adjacency
            bool touchRight = (a.right == b.left);
            bool touchLeft = (a.left == b.right);
            bool yOverlap = (a.top > b.bottom && a.bottom < b.top);

            bool touchTop = (a.top == b.bottom);
            bool touchBottom = (a.bottom == b.top);
            bool xOverlap = (a.right > b.left && a.left < b.right);

            if (((touchRight || touchLeft) && yOverlap) || ((touchTop || touchBottom) && xOverlap))
            {
                rooms[i]->AddNeighbour(rooms[j].get());
                rooms[j]->AddNeighbour(rooms[i].get());
            }
        }
    }
}