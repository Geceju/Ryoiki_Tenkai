#pragma once
#include "AEEngine.h"
#include <array>

class Character {
private:
    int gridX;           
    int gridY;           
    int tileSize;        
    AEGfxVertexList* pMesh; 

    // Movement timing
    float moveTimer;     // Timer to control movement speed
    float moveDelay;     // Delay between moves

public:
    // Constructor
    Character(int startX, int startY, int tile);

    // Destructor
    ~Character();

    void Load();

    void Unload();

    // Update character
    void Update(const std::array<std::array<int, 20>, 15>& maze);

    // Draw character
    void Draw();

    // Getters
    int GetGridX() const { return gridX; }
    int GetGridY() const { return gridY; }

    // Setters
    void SetPosition(int x, int y);
    void SetMoveDelay(float delay) { moveDelay = delay; }

private:
    // Check if a position is valid (not a wall)
    bool IsValidPosition(int x, int y, const std::array<std::array<int, 20>, 15>& maze);
};