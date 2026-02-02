#ifndef ROOM_GENERATOR_H
#define ROOM_GENERATOR_H

#include <vector>
#include <memory>
#include "Room.h"

class RoomGenerator
{
public:
    // Main generation entry point: creates a grid of rooms centered at 0,0
    std::vector<std::unique_ptr<Room>> Generate(int width, int height, int roomSize);

private:
    // Internal helper to link adjacent rooms for the discovery system
    void FindNeighbours(std::vector<std::unique_ptr<Room>>& rooms);
};

#endif