#ifndef JOGO_H
#define JOGO_H

#include "AEEngine.h"
#include <vector>
#include <memory>
#include "Room.h" 

class ItemsManager;

// Handles player movement collision and rendering
class Character
{
public:
    // Constructor taking grid coordinates and tile size
    Character(int startX, int startY, float tile);

    // Destructor to clean up resources
    ~Character();

    // Create the player mesh
    void Load();

    // Release the player mesh
    void Unload();

    // Core loop functions
    // Update logic accepts room list for tile collision checking
    void Update(const std::vector<std::unique_ptr<Room>>& rooms);

    // Renders the player mesh
    void Draw();

    // Set position directly using grid coordinates
    void SetPosition(int x, int y);

    // Getters for position
    int GetGridX() const { return gridX; }
    int GetGridY() const { return gridY; }
    float GetWorldX() const { return worldX; }
    float GetWorldY() const { return worldY; }

    // Logic for item interaction
    void CollectItem(ItemsManager& itemsManager);

private:
    // Checks specific tile value at coordinates
    // Return true only if the tile is 0 meaning Floor
    bool IsPositionWalkable(float x, float y, const std::vector<std::unique_ptr<Room>>& rooms);

    // Grid tracking
    int gridX;
    int gridY;
    float tileSize;

    // Animation and Movement variables
    float worldX;
    float worldY;
    float moveSpeed;
    float moveTimer;
    bool isMoving;

    // Visual mesh
    AEGfxVertexList* pMesh;
};

#endif