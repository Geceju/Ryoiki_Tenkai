#pragma once 
#include "AEEngine.h"       
#include "RoomGenerator.h"   
#include <vector>
#include <memory>            

enum class EnemyState { IDLE, CHASE, RETURN };

class SimpleEnemy {
public:
    SimpleEnemy();
    ~SimpleEnemy();

    void Load();
    void Unload();
    void Update(float playerX, float playerY, float dt, const std::vector<std::unique_ptr<Room>>& rooms);
    void Draw();
    void SetPosition(float x, float y);

private:
    float worldX, worldY;
    float startX, startY;
    float speed;
    float detectionRange;
    float giveUpRange;


    EnemyState currentState;
    struct AEGfxVertexList* pMesh;

    // --- NEW: A* Pathfinding Data ---
    std::vector<AEVec2> currentPath;
    int currentPathIndex;
    float pathRecalculateTimer;

    bool IsPosValid(float x, float y, const std::vector<std::unique_ptr<Room>>& rooms);

    // --- NEW: A* Helper Methods ---
    Room* GetRoomFromPos(float x, float y, const std::vector<std::unique_ptr<Room>>& rooms);
    void CalculateAStarPath(float targetX, float targetY, const std::vector<std::unique_ptr<Room>>& rooms);
};