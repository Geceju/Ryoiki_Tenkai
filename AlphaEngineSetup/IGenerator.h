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

    // UPDATED: We changed the signature to match our Grid system (3 parameters).
    // The "= 0" means any class inheriting from this MUST have this exact function.
    virtual std::vector<std::unique_ptr<Room>> Generate(int w, int h, int roomSize) = 0;

    //// Pure virtual function that forces derived classes to implement a room list generator
    //virtual std::vector<std::unique_ptr<Room>> Generate(int w, int h, int min, int max) = 0;
};

#endif