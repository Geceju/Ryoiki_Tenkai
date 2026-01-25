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

    // Pure virtual function that forces derived classes to implement a room list generator
    virtual std::vector<std::unique_ptr<Room>> Generate(int w, int h, int min, int max) = 0;
};

#endif