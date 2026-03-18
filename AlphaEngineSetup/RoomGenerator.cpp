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

std::vector<std::unique_ptr<Room>> RoomGenerator::Generate(int width, int height, int roomSize)
{
    TilesetManager::Load();

    // load predefined structural layouts
    TemplateManager::LoadTemplates();

    std::vector<std::unique_ptr<Room>> rooms;

    auto now = std::chrono::steady_clock::now();
    Random::Init(static_cast<uint32_t>(now.time_since_epoch().count()));

    int halfW = width / 2;
    int halfH = height / 2;

    // construct grid of generic blocks
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

            rooms.push_back(std::move(newRoom));
        }
    }

    // map adjacency logically
    FindNeighbours(rooms);

    if (!rooms.empty())
    {
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

                // compile unvisited adjacent blocks
                for (auto* neighbor : current->GetNeighbours())
                {
                    if (visited.find(neighbor) == visited.end())
                    {
                        unvisitedNeighbors.push_back(neighbor);
                    }
                }

                if (!unvisitedNeighbors.empty())
                {
                    // pick random adjacent block
                    int idx = Random::Range(0, (int)unvisitedNeighbors.size() - 1);
                    Room* next = unvisitedNeighbors[idx];

                    // record successful path connection
                    current->connectedRooms.push_back(next);
                    next->connectedRooms.push_back(current);

                    // mark visited
                    visited.insert(next);
                    stack.push(next);
                }
                else
                {
                    // reverse track upon dead end
                    stack.pop();
                }
            }

            // attach extra loops to increase multiple solutions
            // increased probability to create more multi door rooms
            for (auto& room : rooms)
            {
                if (Random::Range(0, 100) < 45)
                {
                    auto& neighbors = room->GetNeighbours();
                    if (!neighbors.empty())
                    {
                        Room* randomNeighbor = neighbors[Random::Range(0, (int)neighbors.size() - 1)];

                        // prevent duplicate door records
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

    // apply formatted layouts
    for (auto& room : rooms)
    {
        int requiredDoors = DOOR_NONE;
        AEVec2 myCenter = room->rect.GetCenter();

        // calculate bitmask tracking active connections
        for (auto* connected : room->connectedRooms)
        {
            AEVec2 theirCenter = connected->rect.GetCenter();

            if (theirCenter.y > myCenter.y) requiredDoors |= DOOR_NORTH;
            else if (theirCenter.y < myCenter.y) requiredDoors |= DOOR_SOUTH;
            else if (theirCenter.x > myCenter.x) requiredDoors |= DOOR_EAST;
            else if (theirCenter.x < myCenter.x) requiredDoors |= DOOR_WEST;
        }

        // load specific template array
        RoomTemplate tmpl = TemplateManager::GetRandomTemplate(requiredDoors);
        room->tileMap = tmpl.layout;
    }

    // designate specialized zones
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

            // strip interior obstacles from start zone
            for (int y = 1; y < 15; ++y)
            {
                for (int x = 1; x < 15; ++x)
                {
                    startRoom->tileMap[y][x] = 0;
                }
            }

            // locate extreme coordinates to find corners
            float minX = FLT_MAX;
            float maxX = FLT_MIN; 
            float minY = FLT_MAX;
            float maxY = FLT_MIN;

            for (const auto& room : rooms)
            {
                AEVec2 currentPos = room->rect.GetCenter();
                if (currentPos.x < minX) minX = currentPos.x;
                if (currentPos.x > maxX) maxX = currentPos.x;
                if (currentPos.y < minY) minY = currentPos.y;
                if (currentPos.y > maxY) maxY = currentPos.y;
            }

            // collect four corner rooms
            std::vector<Room*> corners;
            for (const auto& room : rooms)
            {
                AEVec2 currentPos = room->rect.GetCenter();

                // check if room matches extreme x and extreme y
                bool isExtremeX = (fabsf(currentPos.x - minX) < 1.0f || fabsf(currentPos.x - maxX) < 1.0f);
                bool isExtremeY = (fabsf(currentPos.y - minY) < 1.0f || fabsf(currentPos.y - maxY) < 1.0f);

                if (isExtremeX && isExtremeY)
                {
                    corners.push_back(room.get());
                }
            }

            // assign boss to random corner
            if (!corners.empty())
            {
                int bossIndex = Random::Range(0, static_cast<int>(corners.size()) - 1);
                corners[bossIndex]->type = RoomType::Boss;
            }
        }
    }

    // post processing to combine rooms by stripping the dividing wall
    for (auto& room : rooms)
    {
        for (auto* connected : room->connectedRooms)
        {
            // prevent processing the same edge twice
            if (room.get() < connected)
            {
                // chance to merge into a larger contiguous space
                if (Random::Range(0, 100) < 30)
                {
                    AEVec2 myCenter = room->rect.GetCenter();
                    AEVec2 theirCenter = connected->rect.GetCenter();

                    if (theirCenter.y > myCenter.y)
                    {
                        // connected is above so clear top wall
                        for (int x = 1; x < 15; ++x)
                        {
                            room->tileMap[0][x] = 0;
                            connected->tileMap[15][x] = 0;
                        }
                    }
                    else if (theirCenter.y < myCenter.y)
                    {
                        // connected is below so clear bottom wall
                        for (int x = 1; x < 15; ++x)
                        {
                            room->tileMap[15][x] = 0;
                            connected->tileMap[0][x] = 0;
                        }
                    }
                    else if (theirCenter.x > myCenter.x)
                    {
                        // connected is right so clear right wall
                        for (int y = 1; y < 15; ++y)
                        {
                            room->tileMap[y][15] = 0;
                            connected->tileMap[y][0] = 0;
                        }
                    }
                    else if (theirCenter.x < myCenter.x)
                    {
                        // connected is left so clear left wall
                        for (int y = 1; y < 15; ++y)
                        {
                            room->tileMap[y][0] = 0;
                            connected->tileMap[y][15] = 0;
                        }
                    }
                }
            }
        }
    }

    return rooms;
}

void RoomGenerator::FindNeighbours(std::vector<std::unique_ptr<Room>>& rooms)
{
    // scan blocks checking coordinate overlaps
    for (size_t i = 0; i < rooms.size(); ++i)
    {
        for (size_t j = i + 1; j < rooms.size(); ++j)
        {
            Rect& a = rooms[i]->rect;
            Rect& b = rooms[j]->rect;

            // verify boundary contact
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