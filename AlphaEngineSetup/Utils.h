#ifndef UTILS_H
#define UTILS_H

#include "AEEngine.h"  
#include <stdint.h>
#include <algorithm>

// A utility class used to define and manipulate 2D axis-aligned boundaries
class Rect
{
public:
    // Boundary coordinates defining the edges of the rectangle
    int left, top, right, bottom;

    // Initializes a rectangle with zero area at the origin
    Rect();

    // Initializes a rectangle with specific edge coordinates
    Rect(int left, int top, int right, int bottom);

    // Calculates the total horizontal distance of the rectangle
    int width() const;

    // Calculates the total vertical distance of the rectangle
    int height() const;

    // Determines the surface area covered by the rectangle
    int area() const;

    // Updates the boundaries of the rectangle in a single call
    void Set(int l, int t, int r, int b);

    // Checks if this rectangle overlaps with another provided rectangle
    bool IsIntersect(const Rect& other) const;

    // Generates a new rectangle representing the overlapping region between two rectangles
    Rect Intersect(const Rect& other) const;

    // Calculates the midpoint of the rectangle as an Alpha Engine compatible vector
    AEVec2 GetCenter() const;

    // Inline helpers to retrieve the integer center coordinates
    int centerX() const { return (left + right) / 2; }
    int centerY() const { return (top + bottom) / 2; }
};

// A collection of helper functions for generating pseudo-random values
namespace Random
{
    // Sets the starting point for the random number sequence
    void Init(uint32_t seed = 0);

    // Returns a random integer between the specified minimum and maximum inclusive
    int Range(int min, int max);

    // Returns a random floating-point value between the specified minimum and maximum
    float Range(float min, float max);
}

// Global utility to create a 1x1 unit square mesh for the Alpha Engine renderer
AEGfxVertexList* CreateSquare();

#endif