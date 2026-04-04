#include <chrono>
#include <cmath>
#include <algorithm>
#include <vector>
#include <memory>
#include <stack>
#include <set>
#include <numeric>
#include "RoomGenerator.h"
#include "utils.h"
#include "Tilesets.h"
#include "RoomTemplate.h"

/**
 * Main function to generate a complete dungeon.
 * It follows a sequence of: Grid Creation -> Pathfinding -> Template Filling -> Special Room Placement -> Post-Processing.
 */
std::vector<std::unique_ptr<Room>> RoomGenerator::Generate(int width, int height, int roomSize)
{
    // Step 1: Initialize the Template Manager to prepare all possible 16x16 tile layouts.
    TemplateManager::LoadTemplates();

    // Container for our final rooms, using unique_ptr for automatic memory management.
    std::vector<std::unique_ptr<Room>> rooms;

    // Step 2: Seed the Random number generator using the system clock so every dungeon is different.
    auto now = std::chrono::steady_clock::now();
    Random::Init(static_cast<uint32_t>(now.time_since_epoch().count()));

    // Calculate center-offsets so the dungeon is built around the world origin (0,0).
    int halfW = width / 2;
    int halfH = height / 2;

    // Step 3: GRID CONSTRUCTION
    // Nested loops create a mathematical grid of "empty" room boxes.
    for (int y = halfH; y > -halfH; y -= roomSize)
    {
        for (int x = -halfW; x < halfW; x += roomSize)
        {
            Rect roomRect;
            // Define the square boundaries for this specific room.
            roomRect.Set(x, y, x + roomSize, y - roomSize);

            auto newRoom = std::make_unique<Room>(roomRect);

            // Assign a random visual style (Desert, Stone, etc.) from the Tileset Manager.
            newRoom->tilesetID = TilesetManager::GetRandom();

            // Set internal resolution: each room is made of 16x16 actual gameplay tiles.
            newRoom->tileCountX = 16;
            newRoom->tileCountY = 16;
            newRoom->tileSize = (float)roomSize / 16.0f;

            // Add the newly created room to our master list.
            rooms.push_back(std::move(newRoom));
        }
    }

    // Step 4: LOGICAL ADJACENCY
    // Scans all rooms to see which ones are physically touching (North, South, East, West).
    FindNeighbours(rooms);

    if (!rooms.empty())
    {
        // Step 5: LOCATING THE START
        // We find the room closest to (0,0) to serve as our anchor point for generation.
        Room* startRoom = nullptr;
        float minDist = 1000000.0f;
        for (const auto& room : rooms)
        {
            AEVec2 c = room->rect.GetCenter();
            float d = sqrtf(c.x * c.x + c.y * c.y);
            if (d < minDist)
            {
                minDist = d;
                startRoom = room.get();
            }
        }

        // Step 6: RANDOM WALK (MAZE GENERATION)
        // Uses a "Depth First Search" algorithm to ensure every room is reachable.
        if (startRoom)
        {
            std::stack<Room*> stack;    // Keeps track of our path for backtracking.
            std::set<Room*> visited;    // Keeps track of which rooms are already in the maze.

            stack.push(startRoom);
            visited.insert(startRoom);

            while (!stack.empty())
            {
                Room* current = stack.top();
                std::vector<Room*> unvisitedNeighbors;

                // Look at all physical neighbors and filter for ones not yet in the maze.
                for (auto* neighbor : current->GetNeighbours())
                {
                    if (visited.find(neighbor) == visited.end())
                    {
                        unvisitedNeighbors.push_back(neighbor);
                    }
                }

                if (!unvisitedNeighbors.empty())
                {
                    // Pick a random unvisited neighbor to "carve" a path toward.
                    int idx = Random::Range(0, (int)unvisitedNeighbors.size() - 1);
                    Room* next = unvisitedNeighbors[idx];

                    // Record the connection (this creates a "door" between these two rooms).
                    current->connectedRooms.push_back(next);
                    next->connectedRooms.push_back(current);

                    // Mark as visited and move the "brush" forward.
                    visited.insert(next);
                    stack.push(next);
                }
                else
                {
                    // If stuck at a dead end, pop the stack to go back and find another path.
                    stack.pop();
                }
            }

            // Step 7: ADDING CYCLES (NON-LINEARITY)
            // A perfect maze only has one path to each room. This loop adds a 45% chance 
            // to add extra doors, making the dungeon feel less like a hallway and more like a map.
            for (auto& room : rooms)
            {
                if (Random::Range(0, 100) < 45)
                {
                    auto& neighbors = room->GetNeighbours();
                    if (!neighbors.empty())
                    {
                        Room* randomNeighbor = neighbors[Random::Range(0, (int)neighbors.size() - 1)];

                        // Only add the door if one doesn't already exist.
                        if (std::find(room->connectedRooms.begin(), room->connectedRooms.end(), randomNeighbor) == room->connectedRooms.end())
                        {
                            room->connectedRooms.push_back(randomNeighbor);
                            randomNeighbor->connectedRooms.push_back(room.get());
                        }
                    }
                }
            }
        }
    }

    // Step 8: TEMPLATE MAPPING (BITMASKING)
    // Now that doors are decided, we fill the rooms with actual tiles.
    for (auto& room : rooms)
    {
        int requiredDoors = DOOR_NONE;
        AEVec2 myCenter = room->rect.GetCenter();

        // Calculate a bitmask based on where doors were created in Step 6 & 7.
        for (auto* connected : room->connectedRooms)
        {
            AEVec2 theirCenter = connected->rect.GetCenter();

            if (theirCenter.y > myCenter.y)      requiredDoors |= DOOR_NORTH;
            else if (theirCenter.y < myCenter.y) requiredDoors |= DOOR_SOUTH;
            else if (theirCenter.x > myCenter.x) requiredDoors |= DOOR_EAST;
            else if (theirCenter.x < myCenter.x) requiredDoors |= DOOR_WEST;
        }

        // Fetch a pre-designed tile layout that matches this specific N/S/E/W door combo.
        RoomTemplate tmpl = TemplateManager::GetRandomTemplate(requiredDoors);
        room->tileMap = tmpl.layout;
    }

    // Step 9: SPECIAL ROOMS (START AND BOSS)
    if (!rooms.empty())
    {
        Room* startRoom = nullptr;
        float minDistanceToCenter = 1000000.0f;

        // Re-calculate the center room to designate as the player spawn.
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
            startRoom->tilesetID = TilesetType::Type_01; // Force a specific theme for the start area.

            // Strip interior obstacles from the start room so the player doesn't spawn inside a wall.
            for (int y = 1; y < 15; ++y)
            {
                for (int x = 1; x < 15; ++x)
                {
                    startRoom->tileMap[y][x] = 0;
                }
            }

            // Find the furthest edges of the entire dungeon grid.
            float minX = FLT_MAX; float maxX = -FLT_MAX;
            float minY = FLT_MAX; float maxY = -FLT_MAX;

            for (const auto& room : rooms)
            {
                AEVec2 currentPos = room->rect.GetCenter();
                if (currentPos.x < minX) minX = currentPos.x;
                if (currentPos.x > maxX) maxX = currentPos.x;
                if (currentPos.y < minY) minY = currentPos.y;
                if (currentPos.y > maxY) maxY = currentPos.y;
            }

            // Find rooms that exist at the intersection of extreme X and extreme Y (the 4 corners).
            std::vector<Room*> corners;
            for (const auto& room : rooms)
            {
                AEVec2 currentPos = room->rect.GetCenter();
                bool isExtremeX = (fabsf(currentPos.x - minX) < 1.0f || fabsf(currentPos.x - maxX) < 1.0f);
                bool isExtremeY = (fabsf(currentPos.y - minY) < 1.0f || fabsf(currentPos.y - maxY) < 1.0f);

                if (isExtremeX && isExtremeY)
                {
                    corners.push_back(room.get());
                }
            }

            // Randomly select one corner to be the Boss Room, maximizing distance from Start.
            if (!corners.empty())
            {
                int bossIndex = Random::Range(0, static_cast<int>(corners.size()) - 1);
                corners[bossIndex]->type = RoomType::Boss;
            }
        }
    }

    // Step 10: ROOM MERGING (THE "CHISEL")
    // This post-processing step randomly removes dividing walls between connected rooms.
    for (auto& room : rooms)
    {
        for (auto* connected : room->connectedRooms)
        {
            // Only process each boundary once by comparing pointer addresses.
            if (room.get() < connected)
            {
                // 30% chance to turn two small rooms into one large "Great Hall".
                if (Random::Range(0, 100) < 30)
                {
                    AEVec2 myCenter = room->rect.GetCenter();
                    AEVec2 theirCenter = connected->rect.GetCenter();

                    // Detect which side the neighbor is on and clear the tiles along that edge.
                    if (theirCenter.y > myCenter.y) // Neighbor is North
                    {
                        for (int x = 1; x < 15; ++x) { room->tileMap[0][x] = 0; connected->tileMap[15][x] = 0; }
                    }
                    else if (theirCenter.y < myCenter.y) // Neighbor is South
                    {
                        for (int x = 1; x < 15; ++x) { room->tileMap[15][x] = 0; connected->tileMap[0][x] = 0; }
                    }
                    else if (theirCenter.x > myCenter.x) // Neighbor is East
                    {
                        for (int y = 1; y < 15; ++y) { room->tileMap[y][15] = 0; connected->tileMap[y][0] = 0; }
                    }
                    else if (theirCenter.x < myCenter.x) // Neighbor is West
                    {
                        for (int y = 1; y < 15; ++y) { room->tileMap[y][0] = 0; connected->tileMap[y][15] = 0; }
                    }
                }
            }
        }
    }

    // Return the final list of fully prepared rooms to the Level Manager.
    return rooms;
}

/**
 * Finds which rooms are physically adjacent to each other.
 */
void RoomGenerator::FindNeighbours(std::vector<std::unique_ptr<Room>>& rooms)
{
    // Compare every room against every other room.
    for (size_t i = 0; i < rooms.size(); ++i)
    {
        for (size_t j = i + 1; j < rooms.size(); ++j)
        {
            Rect& a = rooms[i]->rect;
            Rect& b = rooms[j]->rect;

            // A neighbor is found if:
            // 1. One room's Right edge touches the other's Left edge (or vice versa).
            // 2. One room's Top edge touches the other's Bottom edge (or vice versa).
            // AND they overlap on the other axis.
            bool touchRight = (a.right == b.left);
            bool touchLeft = (a.left == b.right);
            bool yOverlap = (a.top > b.bottom && a.bottom < b.top);

            bool touchTop = (a.top == b.bottom);
            bool touchBottom = (a.bottom == b.top);
            bool xOverlap = (a.right > b.left && a.left < b.right);

            if (((touchRight || touchLeft) && yOverlap) || ((touchTop || touchBottom) && xOverlap))
            {
                // Register the neighbor link in both directions.
                rooms[i]->AddNeighbour(rooms[j].get());
                rooms[j]->AddNeighbour(rooms[i].get());
            }
        }
    }
}