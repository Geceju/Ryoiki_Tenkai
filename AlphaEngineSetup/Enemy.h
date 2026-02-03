#pragma once 
#include "AEEngine.h"       
#include "RoomGenerator.h"   
#include <vector>
#include <memory>            


enum class EnemyState
{
    IDLE,
    CHASE,
    RETURN
};
class SimpleEnemy
{
public:
    // Current position
    float worldX, worldY;

    // Home position (where it returns to)
    float startX, startY;

    // Settings
    float speed;
    float detectionRange; // How close to start chasing
    float giveUpRange;    // How far before giving up

    // Current State
    EnemyState currentState;

    // Graphics
    AEGfxVertexList* pMesh;

    SimpleEnemy();
    ~SimpleEnemy();

    void Load();
    void Unload();

    // We update SetPosition to save the "Start" location too
    void SetPosition(float x, float y);

    void Update(float playerX, float playerY, float dt, const std::vector<std::unique_ptr<Room>>& rooms);
    void Draw();

private:
    bool IsPosValid(float x, float y, const std::vector<std::unique_ptr<Room>>& rooms);
};