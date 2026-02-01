#include "RoomGenerator.h"
#include "utils.h"
#include <chrono>
#include <cmath> // Included for distance calculations

std::vector<std::unique_ptr<Room>> RoomGenerator::Generate(int width, int height, int roomSize)
{
    std::vector<std::unique_ptr<Room>> rooms;

    auto now = std::chrono::steady_clock::now();
    Random::Init(static_cast<uint32_t>(now.time_since_epoch().count()));

    int halfW = width / 2;
    int halfH = height / 2;

    // GRID LOOP: Fills the area (e.g., 4096x4096) with square rooms (e.g., 256x256)
    for (int y = halfH; y > -halfH; y -= roomSize)
    {
        for (int x = -halfW; x < halfW; x += roomSize)
        {
            Rect roomRect;
            // Set(left, top, right, bottom)
            roomRect.Set(x, y, x + roomSize, y - roomSize);
            rooms.push_back(std::make_unique<Room>(roomRect));
        }
    }

    FindNeighbours(rooms);

    if (!rooms.empty())
    {
        Room* startRoom = nullptr;
        float minDistanceToCenter = 1000000.0f;

        // Identify room closest to (0,0)
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

            // Identify Boss room (furthest from start)
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
    int wallThreshold = 2;
    for (size_t i = 0; i < rooms.size(); ++i)
    {
        for (size_t j = i + 1; j < rooms.size(); ++j)
        {
            Rect expandedA = rooms[i]->rect;
            expandedA.left -= wallThreshold;
            expandedA.right += wallThreshold;
            expandedA.top += wallThreshold;
            expandedA.bottom -= wallThreshold;

            if (expandedA.IsIntersect(rooms[j]->rect))
            {
                rooms[i]->AddNeighbour(rooms[j].get());
                rooms[j]->AddNeighbour(rooms[i].get());
            }
        }
    }
}

//std::vector<std::unique_ptr<Room>> RoomGenerator::Generate(int width, int height, int minimumRoomSize, int maximumRoomSize)
//{
//    std::vector<std::unique_ptr<Room>> rooms;
//    this->minRoomSize = minimumRoomSize;
//    this->maxRoomSize = maximumRoomSize;
//
//    auto now = std::chrono::steady_clock::now();
//    Random::Init(static_cast<uint32_t>(now.time_since_epoch().count()));
//
//    int halfW = width / 2;
//    int halfH = height / 2;
//    Rect baseRect(-halfW, halfH, halfW, -halfH);
//
//    GenerateRooms(baseRect, rooms);
//    FindNeighbours(rooms);
//
//    if (!rooms.empty())
//    {
//        // FIND THE CENTER SPAWN ROOM
//        Room* startRoom = nullptr;
//        float minDistanceToCenter = 1000000.0f; // Start with a very large number
//
//        for (const auto& room : rooms)
//        {
//            AEVec2 center = room->rect.GetCenter();
//            // Calculate distance to (0,0) using Pythagorean theorem: a^2 + b^2 = c^2
//            float dist = sqrtf(center.x * center.x + center.y * center.y);
//
//            if (dist < minDistanceToCenter)
//            {
//                minDistanceToCenter = dist;
//                startRoom = room.get();
//            }
//        }
//
//        // Set the room closest to (0,0) as the Start
//        if (startRoom)
//        {
//            startRoom->type = RoomType::Start;
//
//            // Move the Start room to the front of the vector so Level_Init finds it easily
//            // Or just ensure Level_Init searches for RoomType::Start
//        }
//
//        // FIND THE BOSS ROOM (Furthest from our new Start room)
//        Room* bossCandidate = nullptr;
//        float maxDistance = -1.0f;
//        AEVec2 startPos = startRoom->rect.GetCenter();
//
//        for (const auto& room : rooms)
//        {
//            if (room.get() == startRoom) continue;
//
//            AEVec2 currentPos = room->rect.GetCenter();
//            float dist = sqrtf(powf(currentPos.x - startPos.x, 2) + powf(currentPos.y - startPos.y, 2));
//
//            if (dist > maxDistance)
//            {
//                maxDistance = dist;
//                bossCandidate = room.get();
//            }
//        }
//
//        if (bossCandidate)
//        {
//            bossCandidate->type = RoomType::Boss;
//        }
//    }
//
//    return rooms;
//}
//
//void RoomGenerator::GenerateRooms(Rect& rect, std::vector<std::unique_ptr<Room>>& rooms)
//{
//    // If the current area exceeds the maximum allowed size, it must be subdivided
//    if (rect.width() > maxRoomSize || rect.height() > maxRoomSize)
//    {
//        Rect r1, r2;
//        if (SplitRect(rect, r1, r2))
//        {
//            GenerateRooms(r1, rooms);
//            GenerateRooms(r2, rooms);
//            return;
//        }
//    }
//
//    // Side-by-side logic: Rooms occupy the full extent of the BSP container
//    rooms.push_back(std::make_unique<Room>(rect));
//}
//
//bool RoomGenerator::SplitRect(Rect& rect, Rect& rect1, Rect& rect2)
//{
//    int w = rect.width();
//    int h = rect.height();
//
//    // We still need to prevent rooms from becoming "slivers" that are too thin to walk through.
//    // If width is twice the minimum but height is at the limit, we must split vertically.
//    if (w > maxRoomSize && h <= 2 * minRoomSize)
//    {
//        SplitVertically(rect, rect1, rect2);
//    }
//    else if (h > maxRoomSize && w <= 2 * minRoomSize)
//    {
//        SplitHorizontally(rect, rect1, rect2);
//    }
//    // STOP CONDITION: If the room is small enough, we stop splitting.
//    // I've adjusted the probability here to allow more variance in room sizes.
//    else if (w < maxRoomSize && h < maxRoomSize && (Random::Range(0.0f, 1.0f) < 0.25f))
//    {
//        return false;
//    }
//    else
//    {
//        // VARIETY LOGIC: Instead of always splitting the longest side, 
//        // we use a weighted random choice. 
//        // This allows for "Long" rooms (vertical splits on short widths) 
//        // and "Wide" rooms (horizontal splits on short heights).
//        if (Random::Range(0.0f, 1.0f) < 0.5f)
//        {
//            // Even if the room is tall, we might split it vertically to make two thin, long rooms.
//            if (w >= 2 * minRoomSize)
//            {
//                SplitVertically(rect, rect1, rect2);
//            }
//            else
//            {
//                SplitHorizontally(rect, rect1, rect2);
//            }
//        }
//        else
//        {
//            if (h >= 2 * minRoomSize)
//            {
//                SplitHorizontally(rect, rect1, rect2);
//            }
//            else
//            {
//                SplitVertically(rect, rect1, rect2);
//            }
//        }
//    }
//
//    return true;
//}
//
//void RoomGenerator::SplitVertically(Rect& rect, Rect& rect1, Rect& rect2)
//{
//    // The split point is chosen randomly between the current left and right edges.
//    // We include the minimum room size as a buffer to prevent creating rooms that are too narrow.
//    int v = Random::Range(rect.left + minRoomSize, rect.right - minRoomSize);
//
//    // We divide the parent rectangle into a left child and a right child.
//    // Both children inherit the original vertical bounds of the parent.
//    rect1.Set(rect.left, rect.top, v, rect.bottom);
//    rect2.Set(v, rect.top, rect.right, rect.bottom);
//}
//
//void RoomGenerator::SplitHorizontally(Rect& rect, Rect& rect1, Rect& rect2)
//{
//    // We choose a horizontal split line between the current bottom and top edges.
//    // Because we use the rect's actual coordinates, this works correctly in negative space.
//    int h = Random::Range(rect.bottom + minRoomSize, rect.top - minRoomSize);
//
//    // The parent rectangle is divided into a top child and a bottom child.
//    // The shared horizontal line (h) becomes the new boundary for both.
//    rect1.Set(rect.left, rect.top, rect.right, h);
//    rect2.Set(rect.left, h, rect.right, rect.bottom);
//}
//
//void RoomGenerator::FindNeighbours(std::vector<std::unique_ptr<Room>>& rooms)
//{
//    // Minimal threshold for detecting contact between perfectly adjacent rectangles
//    int wallThreshold = 2;
//
//    for (size_t i = 0; i < rooms.size(); ++i)
//    {
//        for (size_t j = i + 1; j < rooms.size(); ++j)
//        {
//            // Expand the detection bounds of the first room to check for contact
//            Rect expandedA = rooms[i]->rect;
//            expandedA.left -= wallThreshold;
//            expandedA.right += wallThreshold;
//            expandedA.top += wallThreshold;
//            expandedA.bottom -= wallThreshold;
//
//            if (expandedA.IsIntersect(rooms[j]->rect))
//            {
//                // Assign bidirectional pointers to establish the neighborhood link
//                rooms[i]->AddNeighbour(rooms[j].get());
//                rooms[j]->AddNeighbour(rooms[i].get());
//            }
//        }
//    }
//}