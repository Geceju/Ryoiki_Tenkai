#pragma once
#ifndef ROOM_GENERATOR_H
#define ROOM_GENERATOR_H

#include "IGenerator.h"
#include <vector>
#include <memory>

// A generator that utilizes recursive subdivision to create non-overlapping rooms
class RoomGenerator : public IGenerator
{
public:
    // Constructs a full list of rooms based on the provided dimensional constraints
    std::vector<std::unique_ptr<Room>> Generate(int width, int height, int roomSize);


private:
    // Post-processing step to link rooms that share a physical border
    void FindNeighbours(std::vector<std::unique_ptr<Room>>& rooms);
};

#endif