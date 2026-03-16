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

    /**
     * @brief Calculates the horizontal distance between the left and right edges.
     * @return The width of the rectangle.
     */
    int width() const;

    /**
     * @brief Calculates the vertical distance between the top and bottom edges.
     * @return The height of the rectangle.
     */
    int height() const;

    /**
     * @brief Determines the total surface area covered by the rectangle.
     * @return The area calculated as width * height.
     */
    int area() const;

    /**
     * @brief Updates all four boundary values of the rectangle simultaneously.
     * @param l The new left edge coordinate.
     * @param t The new top edge coordinate.
     * @param r The new right edge coordinate.
     * @param b The new bottom edge coordinate.
     */
    void Set(int l, int t, int r, int b);

    /**
     * @brief Performs a standard AABB collision check against another rectangle.
     * @param other The rectangle to check for overlap against.
     * @return True if the rectangles overlap, otherwise false.
     */
    bool IsIntersect(const Rect& other) const;

    /**
     * @brief Generates a new geometric rectangle representing the exact overlapping region.
     * @param other The intersecting rectangle.
     * @return A new Rect defined by the bounds of the overlap.
     */
    Rect Intersect(const Rect& other) const;

    /**
     * @brief Calculates the floating-point midpoint of the rectangle.
     * @return An Alpha Engine 2D vector (AEVec2) representing the absolute center.
     */
    AEVec2 GetCenter() const;

    // Inline helpers to retrieve the integer center coordinates
    int centerX() const { return (left + right) / 2; }
    int centerY() const { return (top + bottom) / 2; }
};

// A collection of helper functions for generating pseudo-random values
namespace Random
{
    /**
     * @brief Seeds the global random number generator.
     * @param seed The initialization seed (defaults to 0).
     */
    void Init(uint32_t seed = 0);

    /**
     * @brief Generates a random integer within a specified inclusive range.
     * @param min The lowest possible value.
     * @param max The highest possible value.
     * @return A random integer.
     */
    int Range(int min, int max);

    /**
     * @brief Generates a random floating-point number within a specified range.
     * @param min The lowest possible value.
     * @param max The highest possible value.
     * @return A random float.
     */
    float Range(float min, float max);
}

/**
 * @brief Constructs a primitive 1x1 unit square mesh centered at the origin.
 * @return A pointer to the compiled AEGfxVertexList mesh stream.
 */
AEGfxVertexList* CreateSquare();

#endif