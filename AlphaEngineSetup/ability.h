//author : Felicia
#ifndef ABILITY_H
#define ABILITY_H

#include "AEEngine.h"
#include "Room.h"
#include <vector>
#include <memory>

class Character;
class SimpleEnemy;
class ItemsManager;

class PlayerAbilities
{
public:
    // constructor destructor
    PlayerAbilities();
    ~PlayerAbilities();

    void Load();
    void Unload();

    /*
    core logic:
    check keyboard input, activate avilities, update timer & guide path(ability 3)
    */
    void Update(float dt,
        Character& player,
        std::vector<SimpleEnemy>& enemy,
        const ItemsManager& items,
        const std::vector<std::unique_ptr<Room>>& rooms);

    void DrawGuide() const;

    void ActivateSpeedBoost(Character& player);
    void ActivateStun(std::vector<SimpleEnemy>& enemy);
    void ActivateGuide(const Character& player,
        const ItemsManager& items,
        const std::vector<std::unique_ptr<Room>>& rooms);

    bool IsSpeedBoostActive() const { return speedBoostTimer > 0.0f; }
    bool IsGuideActive() const { return guideTimer > 0.0f && !guidePathPoints.empty(); }

private:
    void CreateGuideMesh();

    void RebuildGuidePath(const Character& player,
        const ItemsManager& items,
        const std::vector<std::unique_ptr<Room>>& rooms);

    bool FindNearestItem(const Character& player,
        const ItemsManager& items,
        float& outX,
        float& outY) const;

    bool WorldToTile(float worldX, float worldY,
        const std::vector<std::unique_ptr<Room>>& rooms,
        int& outTileX, int& outTileY) const;

    AEVec2 TileToWorld(int tileX, int tileY, float tileSize) const;

    bool IsTileWalkable(int tileX, int tileY,
        const std::vector<std::unique_ptr<Room>>& rooms,
        float tileSize) const;

private:
    float speedBoostTimer;
    float guideTimer;

    float basePlayerSpeed;
    bool speedBoostApplied;

    bool prevKey1;
    bool prevKey2;
    bool prevKey3;

    float pathRefreshTimer;
    std::vector<AEVec2> guidePathPoints;

    float guideOrbDistFromItem;
    float guidePathTotalLength;

    AEGfxVertexList* pGuideMesh;
};

#endif