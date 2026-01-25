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
    RoomGenerator() = default;
    ~RoomGenerator() override = default;

    // Constructs a full list of rooms based on the provided dimensional constraints
    std::vector<std::unique_ptr<Room>> Generate(int width, int height, int minimumRoomSize, int maximumRoomSize) override;

private:
    int minRoomSize = 5;
    int maxRoomSize = 5;

    // Core recursive function that manages the subdivision of the world space
    void GenerateRooms(Rect& rect, std::vector<std::unique_ptr<Room>>& rooms);

    // Decision logic to determine if a rectangle can and should be split
    bool SplitRect(Rect& rect, Rect& rect1, Rect& rect2);

    // Helper to divide a space along a random vertical line
    void SplitVertically(Rect& rect, Rect& rect1, Rect& rect2);

    // Helper to divide a space along a random horizontal line
    void SplitHorizontally(Rect& rect, Rect& rect1, Rect& rect2);

    // Post-processing step to link rooms that share a physical border
    void FindNeighbours(std::vector<std::unique_ptr<Room>>& rooms);
};

#endif