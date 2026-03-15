#include "ability.h"
#include "jogo.h"
#include "Enemy.h"
#include "Items.h"
#include <cmath>
#include <queue>
#include <map>
#include <algorithm>

PlayerAbilities::PlayerAbilities()
    : speedBoostTimer(0.0f),
    guideTimer(0.0f),
    basePlayerSpeed(0.0f),
    speedBoostApplied(false),
    prevKey1(false),
    prevKey2(false),
    prevKey3(false),
    pathRefreshTimer(0.0f),
    pGuideMesh(nullptr)
{
}

PlayerAbilities::~PlayerAbilities()
{
    Unload();
}

void PlayerAbilities::Load()
{
    if (!pGuideMesh)
    {
        CreateGuideMesh();
    }
}

void PlayerAbilities::Unload()
{
    guidePathPoints.clear();

    if (pGuideMesh)
    {
        AEGfxMeshFree(pGuideMesh);
        pGuideMesh = nullptr;
    }
}

void PlayerAbilities::CreateGuideMesh()
{
    AEGfxMeshStart();

    // A thin horizontal rectangle centered at origin.
    // It will be scaled to the needed length and rotated toward the target item.
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

    pGuideMesh = AEGfxMeshEnd();
}

void PlayerAbilities::ActivateSpeedBoost(Character& player)
{
    if (!speedBoostApplied)
    {
        basePlayerSpeed = player.GetMoveSpeed();
        player.SetMoveSpeed(basePlayerSpeed * 2.0f);
        speedBoostApplied = true;
    }

    speedBoostTimer = 30.0f;
}

void PlayerAbilities::ActivateStun(SimpleEnemy& enemy)
{
    enemy.Stun(10.0f);
}

bool PlayerAbilities::FindNearestItem(const Character& player,
    const ItemsManager& items,
    float& outX,
    float& outY) const
{
    const std::vector<Item>& allItems = items.GetItems();

    bool found = false;
    float bestDistSq = 0.0f;

    for (const Item& item : allItems)
    {
        if (item.collected || !item.active)
            continue;

        float dx = item.x - player.GetWorldX();
        float dy = item.y - player.GetWorldY();
        float distSq = dx * dx + dy * dy;

        if (!found || distSq < bestDistSq)
        {
            found = true;
            bestDistSq = distSq;
            outX = item.x;
            outY = item.y;
        }
    }

    return found;
}

bool PlayerAbilities::WorldToTile(float worldX, float worldY,
    const std::vector<std::unique_ptr<Room>>& rooms,
    int& outTileX, int& outTileY) const
{
    for (const auto& room : rooms)
    {
        if (worldX >= room->rect.left && worldX <= room->rect.right &&
            worldY >= room->rect.bottom && worldY <= room->rect.top)
        {
            float tileSize = room->tileSize;

            int localTileX = static_cast<int>((worldX - room->rect.left) / tileSize);
            int localTileY = static_cast<int>((room->rect.top - worldY) / tileSize);

            int worldBaseTileX = static_cast<int>(room->rect.left / tileSize);
            int worldBaseTileY = static_cast<int>(room->rect.bottom / tileSize);

            outTileX = worldBaseTileX + localTileX;
            outTileY = worldBaseTileY + localTileY;
            return true;
        }
    }

    return false;
}

AEVec2 PlayerAbilities::TileToWorld(int tileX, int tileY, float tileSize) const
{
    AEVec2 result;
    result.x = (tileX * tileSize) + (tileSize * 0.5f);
    result.y = (tileY * tileSize) + (tileSize * 0.5f);
    return result;
}

bool PlayerAbilities::IsTileWalkable(int tileX, int tileY,
    const std::vector<std::unique_ptr<Room>>& rooms,
    float tileSize) const
{
    float worldX = (tileX * tileSize) + (tileSize * 0.5f);
    float worldY = (tileY * tileSize) + (tileSize * 0.5f);

    for (const auto& room : rooms)
    {
        if (worldX >= room->rect.left && worldX <= room->rect.right &&
            worldY >= room->rect.bottom && worldY <= room->rect.top)
        {
            int localTileX = static_cast<int>((worldX - room->rect.left) / tileSize);
            int localTileY = static_cast<int>((room->rect.top - worldY) / tileSize);
            return room->GetTile(localTileX, localTileY) == 0;
        }
    }

    return false;
}

void PlayerAbilities::RebuildGuidePath(const Character& player,
    const ItemsManager& items,
    const std::vector<std::unique_ptr<Room>>& rooms)
{
    guidePathPoints.clear();

    float targetX = 0.0f;
    float targetY = 0.0f;

    if (!FindNearestItem(player, items, targetX, targetY))
        return;

    int startTileX = 0, startTileY = 0;
    int goalTileX = 0, goalTileY = 0;

    if (!WorldToTile(player.GetWorldX(), player.GetWorldY(), rooms, startTileX, startTileY))
        return;

    if (!WorldToTile(targetX, targetY, rooms, goalTileX, goalTileY))
        return;

    float tileSize = rooms.empty() ? 32.0f : rooms[0]->tileSize;

    struct TileNode
    {
        int x, y;
        bool operator<(const TileNode& other) const
        {
            if (x != other.x) return x < other.x;
            return y < other.y;
        }
    };

    TileNode start{ startTileX, startTileY };
    TileNode goal{ goalTileX, goalTileY };

    std::queue<TileNode> frontier;
    std::map<TileNode, TileNode> cameFrom;
    std::map<TileNode, bool> visited;

    frontier.push(start);
    visited[start] = true;

    const int dirX[4] = { 1, -1, 0, 0 };
    const int dirY[4] = { 0, 0, 1, -1 };

    bool found = false;

    while (!frontier.empty())
    {
        TileNode current = frontier.front();
        frontier.pop();

        if (current.x == goal.x && current.y == goal.y)
        {
            found = true;
            break;
        }

        for (int i = 0; i < 4; ++i)
        {
            TileNode next{ current.x + dirX[i], current.y + dirY[i] };

            if (visited[next])
                continue;

            if (!IsTileWalkable(next.x, next.y, rooms, tileSize))
                continue;

            visited[next] = true;
            cameFrom[next] = current;
            frontier.push(next);
        }
    }

    if (!found)
        return;

    std::vector<AEVec2> reversedPoints;
    TileNode step = goal;

    reversedPoints.push_back(TileToWorld(step.x, step.y, tileSize));

    while (!(step.x == start.x && step.y == start.y))
    {
        step = cameFrom[step];
        reversedPoints.push_back(TileToWorld(step.x, step.y, tileSize));
    }

    std::reverse(reversedPoints.begin(), reversedPoints.end());

    guidePathPoints.push_back({ player.GetWorldX(), player.GetWorldY() });
    for (const AEVec2& p : reversedPoints)
    {
        guidePathPoints.push_back(p);
    }
}

void PlayerAbilities::ActivateGuide(const Character& player,
    const ItemsManager& items,
    const std::vector<std::unique_ptr<Room>>& rooms)
{
    guideTimer = 20.0f;
    pathRefreshTimer = 0.0f;
    RebuildGuidePath(player, items, rooms);
}

void PlayerAbilities::Update(float dt,
    Character& player,
    SimpleEnemy& enemy,
    const ItemsManager& items,
    const std::vector<std::unique_ptr<Room>>& rooms)
{
    bool key1 = AEInputCheckCurr(AEVK_1) != 0;
    bool key2 = AEInputCheckCurr(AEVK_2) != 0;
    bool key3 = AEInputCheckCurr(AEVK_3) != 0;

    if (key1 && !prevKey1)
    {
        ActivateSpeedBoost(player);
    }

    if (key2 && !prevKey2)
    {
        ActivateStun(enemy);
    }

    if (key3 && !prevKey3)
    {
        ActivateGuide(player, items, rooms);
    }

    prevKey1 = key1;
    prevKey2 = key2;
    prevKey3 = key3;

    if (speedBoostTimer > 0.0f)
    {
        speedBoostTimer -= dt;

        if (speedBoostTimer <= 0.0f)
        {
            speedBoostTimer = 0.0f;

            if (speedBoostApplied)
            {
                player.SetMoveSpeed(basePlayerSpeed);
                speedBoostApplied = false;
            }
        }
    }

    if (guideTimer > 0.0f)
    {
        guideTimer -= dt;
        pathRefreshTimer -= dt;

        if (pathRefreshTimer <= 0.0f)
        {
            pathRefreshTimer = 0.25f;
            RebuildGuidePath(player, items, rooms);
        }

        if (guideTimer <= 0.0f)
        {
            guideTimer = 0.0f;
            guidePathPoints.clear();
        }
    }
}

void PlayerAbilities::DrawGuide() const
{
    if (!pGuideMesh || guidePathPoints.size() < 2)
    {
        return;
    }

    for (size_t i = 0; i + 1 < guidePathPoints.size(); ++i)
    {
        const AEVec2& a = guidePathPoints[i];
        const AEVec2& b = guidePathPoints[i + 1];

        float dx = b.x - a.x;
        float dy = b.y - a.y;
        float length = std::sqrt(dx * dx + dy * dy);

        if (length <= 0.001f)
        {
            continue;
        }

        float angle = std::atan2(dy, dx);
        float midX = (a.x + b.x) * 0.5f;
        float midY = (a.y + b.y) * 0.5f;

        AEMtx33 scale, rot, trans, temp, finalMtx;

        AEMtx33Scale(&scale, length, 4.0f);
        AEMtx33Rot(&rot, angle);
        AEMtx33Trans(&trans, midX, midY);

        AEMtx33Concat(&temp, &rot, &scale);
        AEMtx33Concat(&finalMtx, &trans, &temp);

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetColorToMultiply(0.0f, 1.0f, 0.0f, 0.8f);
        AEGfxSetTransform(finalMtx.m);
        AEGfxMeshDraw(pGuideMesh, AE_GFX_MDM_TRIANGLES);
    }
}