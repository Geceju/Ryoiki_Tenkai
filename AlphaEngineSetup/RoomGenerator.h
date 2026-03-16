#ifndef ROOM_GENERATOR_H
#define ROOM_GENERATOR_H

#include <vector>
#include <memory>
#include "Room.h"

class RoomGenerator
{
public:
    /**
     * @brief Generates a fully interconnected dungeon layout composed of distinct rooms.
     * @param width The total maximum width of the allowable dungeon area.
     * @param height The total maximum height of the allowable dungeon area.
     * @param roomSize The physical dimension (width and height) of a single room block.
     * @return A vector of uniquely owned Room objects forming the dungeon.
     */
    std::vector<std::unique_ptr<Room>> Generate(int width, int height, int roomSize);

private:
    /**
     * @brief Scans all generated rooms to detect and register physical adjacency.
     * @param rooms A reference to the vector containing all generated dungeon rooms.
     */
    void FindNeighbours(std::vector<std::unique_ptr<Room>>& rooms);
};

#endif