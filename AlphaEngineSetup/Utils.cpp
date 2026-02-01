#include "Utils.h"

// ========================================================
// RECT IMPLEMENTATION
// ========================================================

Rect::Rect()
    : left(0), top(0), right(0), bottom(0)
{
    // Default constructor initializes a rectangle with no area at the origin
}

Rect::Rect(int left, int top, int right, int bottom)
{
    // Overloaded constructor allows for immediate definition of rectangle boundaries
    Set(left, top, right, bottom);
}

int Rect::width() const
{
    // Calculate the horizontal distance between the left and right edges
    return right - left;
}

int Rect::height() const
{
    // Subtract the smaller number (bottom) from the larger number (top)
    return top - bottom;
}

int Rect::area() const
{
    // Determine the total surface area by multiplying the dimensions
    return width() * height();
}

void Rect::Set(int l, int t, int r, int b)
{
    // Update all four boundary values simultaneously to define a new rectangular area
    left = l;
    top = t;
    right = r;
    bottom = b;
}

bool Rect::IsIntersect(const Rect& other) const
{
    // Perform a standard AABB collision check to see if two rectangles overlap in 2D space
    // An intersection occurs if all boundary conditions are met simultaneously
    return (left < other.right && right > other.left &&
        top > other.bottom && bottom < other.top);
}



Rect Rect::Intersect(const Rect& other) const
{
    // Construct a new rectangle representing the shared overlapping region between two shapes.
    // We cast to float to use AE math macros, then cast back to int to store in the Rect.
    return Rect(
        static_cast<int>(AEMax(static_cast<float>(left), static_cast<float>(other.left))),
        static_cast<int>(AEMax(static_cast<float>(top), static_cast<float>(other.top))),
        static_cast<int>(AEMin(static_cast<float>(right), static_cast<float>(other.right))),
        static_cast<int>(AEMin(static_cast<float>(bottom), static_cast<float>(other.bottom)))
    );
}

AEVec2 Rect::GetCenter() const
{
    AEVec2 center;

    // Calculate the midpoint. We cast to float first to ensure floating-point division
    // occurs, preventing any integer truncation during the average calculation.
    float centerX = static_cast<float>(left + right) * 0.5f;
    float centerY = static_cast<float>(top + bottom) * 0.5f;

    // Package the results into an Alpha Engine vector structure
    AEVec2Set(&center, centerX, centerY);

    return center;
}

// ========================================================
// RANDOM IMPLEMENTATION
// ========================================================

namespace Random
{
    void Init(uint32_t seed)
    {
        // Seed the global random number generator
        srand(seed);
    }

    int Range(int min, int max)
    {
        // Guard against logical errors if the minimum and maximum values are accidentally reversed
        if (min > max)
        {
            std::swap(min, max);
        }

        // AERandFloat() returns a float between 0.0 and 1.0. 
        // We cast the range to float for the math, then truncate to int for the result.
        float range = static_cast<float>(max - min + 1);
        return min + static_cast<int>(AERandFloat() * range);
    }

    float Range(float min, float max)
    {
        // Guard against logical errors if the parameters are swapped
        if (min > max)
        {
            std::swap(min, max);
        }

        // Return a floating-point value between the minimum and maximum bounds
        return min + (AERandFloat() * (max - min));
    }
}

// ========================================================
// SQUARE IMPLEMENTATION
// ========================================================

AEGfxVertexList* CreateSquare()
{
    // Initialize a new mesh stream to define the vertices of a primitive square
    AEGfxMeshStart();

    // Assemble a 1x1 unit square centered at the origin.
    // These values are already float literals, so no casting is required here.
    AEGfxVertexAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
    AEGfxVertexAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f);
    AEGfxVertexAdd(-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

    AEGfxVertexAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f);
    AEGfxVertexAdd(0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f);
    AEGfxVertexAdd(-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

    // Finalize the mesh and return the pointer
    return AEGfxMeshEnd();
}