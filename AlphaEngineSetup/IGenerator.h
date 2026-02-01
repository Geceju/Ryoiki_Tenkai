#pragma once

#ifndef IGENERATOR_H
#define IGENERATOR_H

#include "Room.h"
#include <vector>
#include <memory>

// Base interface for all procedural generation algorithms
class IGenerator
{
public:
    // Virtual destructor ensures proper cleanup of derived generator classes
    virtual ~IGenerator()
    {
    }

    // Generates a randomized dungeon layout within a wxh area using a fixed roomSize; returns ownership of Room objects via unique_ptrs
    virtual std::vector<std::unique_ptr<Room>> Generate(int w, int h, int roomSize) = 0;
};

#endif