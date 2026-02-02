#ifndef CHARACTER_H
#define CHARACTER_H

#include "AEEngine.h"
#include <vector>
#include <memory>
#include "Room.h" 

class ItemsManager;

class Character
{
public:
    Character(int startX, int startY, float tile);
    ~Character();

    void Load();
    void Unload();

    // Core loop functions
    void Update(const std::vector<std::unique_ptr<Room>>& rooms);
    void Draw();

    // Getters and Setters
    void SetPosition(int x, int y);
    int GetGridX() const { return gridX; }
    int GetGridY() const { return gridY; }
    float GetWorldX() const { return worldX; }
    float GetWorldY() const { return worldY; }

    void CollectItem(ItemsManager& itemsManager);

private:
    // Helper for free-roam collision detection
    bool IsPointInsideAnyRoom(float x, float y, const std::vector<std::unique_ptr<Room>>& rooms);

    // Grid tracking
    int gridX;
    int gridY;
    float tileSize;

    // Animation and Movement
    float worldX;       // Visual X position in world space
    float worldY;       // Visual Y position in world space
    float moveSpeed;    // Pixels per second
    float moveTimer;    // Drives the sine-wave walking animation
    bool isMoving;      // Tracks if the player is currently providing input

    AEGfxVertexList* pMesh;
};

#endif