#pragma once
#include "AEEngine.h"
#include <array>

class ItemsManager;

class Character {
private:
    int gridX;           
    int gridY;           
    float tileSize;        
    AEGfxVertexList* pMesh; 

    // Movement timing
    float moveTimer;     // Timer to control movement speed
    float moveDelay;     // Delay between moves

public:
    // Constructor
    Character(int startX, int startY, float tile);

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

    // Item Collection
    float GetWorldX() const { return static_cast<float>(gridX); }
    float GetWorldY() const { return static_cast<float>(gridY); }

    // Function for item collection with E key
    void CollectItem(ItemsManager& itemsManager);


    // Setters
    void SetPosition(int x, int y);
    void SetMoveDelay(float delay) { moveDelay = delay; }

private:
    // Check if a position is valid (not a wall)
    bool IsValidPosition(int x, int y, const std::array<std::array<int, 20>, 15>& maze);
};