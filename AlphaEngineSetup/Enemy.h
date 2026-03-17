#pragma once 
#include "AEEngine.h"       
#include "RoomGenerator.h"   
#include <vector>
#include <memory>            

enum class EnemyState { IDLE, CHASE, RETURN, PATROL };

class SimpleEnemy {
public:
    SimpleEnemy();
    ~SimpleEnemy();

    void Load();
    void Unload();
    void Update(float playerX, float playerY, float dt, const std::vector<std::unique_ptr<Room>>& rooms);
    void Draw();
    void SetPosition(float x, float y);
    void SetChaseDuration(float duration) { maxChaseTime = duration; }
    float GetWorldX() const { return worldX; }
    float GetWorldY() const { return worldY; }
    void SetState(EnemyState newState) { currentState = newState; }

private:
    float worldX, worldY;
    float startX, startY;
    float speed;
    float detectionRange;
    float giveUpRange;
    float chaseTimer;
    float maxChaseTime;
    bool hasPatrolTarget;

public:
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
    bool IsPositionWalkable(float x, float y, const std::vector<std::unique_ptr<Room>>& rooms);
};
