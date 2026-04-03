#include "ability.h"
#include "jogo.h"
#include "Enemy.h"
#include "Items.h"
#include "Inventory.h"
#include "AudioSystem.h"
#include <cmath>
#include <queue>
#include <map>
#include <algorithm>

/*
handles 3 abilties:
1. speed boost -> increases player speed
2. stun enemies -> stun all enemies
3. guide line to nearest item
*/

// Constructors
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

// Destructor
PlayerAbilities::~PlayerAbilities()
{
    Unload();
}

// mesh for guide line
void PlayerAbilities::Load()
{
    if (!pGuideMesh)
    {
        CreateGuideMesh();
    }
}

// mesh free logic
void PlayerAbilities::Unload()
{
    guidePathPoints.clear();

    if (pGuideMesh)
    {
        AEGfxMeshFree(pGuideMesh);
        pGuideMesh = nullptr;
    }
}

// resuable triangle mesh for guide lines
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

// Ability 1 (speed boost)
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

// Ability 2 (stun enemy)
void PlayerAbilities::ActivateStun(std::vector<SimpleEnemy>& enemy)
{
    for (auto& e : enemy)
    {
        e.Stun(10.0f);
    }
}

// Ability 3 helper (kept for legacy/fallback)
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

// --- FIXED: world coordinates -> global tile grid ---
bool PlayerAbilities::WorldToTile(float worldX, float worldY,
    const std::vector<std::unique_ptr<Room>>& rooms,
    int& outTileX, int& outTileY) const
{
    if (rooms.empty()) return false;

    float tileSize = rooms[0]->tileSize;
    outTileX = static_cast<int>(worldX / tileSize);
    outTileY = static_cast<int>(worldY / tileSize);
    return true;
}

// --- FIXED: tile grid -> global world coordinates ---
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
    if (rooms.empty()) return;

    float tileSize = rooms[0]->tileSize;
    int startTileX = static_cast<int>(player.GetWorldX() / tileSize);
    int startTileY = static_cast<int>(player.GetWorldY() / tileSize);

    struct TileNode
    {
        int x, y;
        bool operator<(const TileNode& other) const {
            if (x != other.x) return x < other.x;
            return y < other.y;
        }
        bool operator==(const TileNode& other) const {
            return x == other.x && y == other.y;
        }
    };

    // Pre-map active items by tile to quickly check if a tile has an item
    std::map<TileNode, AEVec2> tileToItemPos;
    bool hasItems = false;
    for (const Item& item : items.GetItems()) {
        if (!item.collected && item.active) {
            TileNode tn{ static_cast<int>(item.x / tileSize), static_cast<int>(item.y / tileSize) };
            tileToItemPos[tn] = { item.x, item.y };
            hasItems = true;
        }
    }

    if (!hasItems) return;

    TileNode start{ startTileX, startTileY };
    std::queue<TileNode> frontier;
    std::map<TileNode, TileNode> cameFrom;
    std::map<TileNode, bool> visited;

    frontier.push(start);
    visited[start] = true;

    const int dirX[4] = { 1, -1, 0, 0 };
    const int dirY[4] = { 0, 0, 1, -1 };

    bool found = false;
    TileNode goalNode = start;

    // --- FIXED: BFS outward from player to find the TRUE shortest walking path ---
    while (!frontier.empty())
    {
        TileNode current = frontier.front();
        frontier.pop();

        // If this tile contains an item, we found the absolute closest one!
        if (tileToItemPos.find(current) != tileToItemPos.end()) {
            found = true;
            goalNode = current;
            break;
        }

        for (int i = 0; i < 4; ++i)
        {
            TileNode next{ current.x + dirX[i], current.y + dirY[i] };

            if (visited[next]) continue;
            if (!IsTileWalkable(next.x, next.y, rooms, tileSize)) continue;

            visited[next] = true;
            cameFrom[next] = current;
            frontier.push(next);
        }
    }

    if (!found) return;

    std::vector<AEVec2> reversedPoints;
    TileNode step = goalNode;

    // Trace path back
    while (!(step == start))
    {
        reversedPoints.push_back(TileToWorld(step.x, step.y, tileSize));
        step = cameFrom[step];
    }

    std::reverse(reversedPoints.begin(), reversedPoints.end());

    // Build the raw path connecting player precisely to the item
    std::vector<AEVec2> rawPath;
    rawPath.push_back({ player.GetWorldX(), player.GetWorldY() });

    if (!reversedPoints.empty()) {
        for (size_t i = 0; i < reversedPoints.size(); ++i) {
            if (i == reversedPoints.size() - 1) {
                rawPath.push_back(tileToItemPos[goalNode]); // Connect exactly to item
            }
            else {
                rawPath.push_back(reversedPoints[i]);
            }
        }
    }
    else {
        rawPath.push_back(tileToItemPos[goalNode]);
    }

    // --- NEW: Path Simplifier (Creates straight lines by removing jagged steps) ---
    if (rawPath.size() > 2) {
        guidePathPoints.push_back(rawPath[0]);

        for (size_t i = 1; i < rawPath.size() - 1; ++i) {
            AEVec2 prev = guidePathPoints.back();
            AEVec2 curr = rawPath[i];
            AEVec2 next = rawPath[i + 1];

            // Use cross product to check if the points form a straight line
            float cross = (curr.x - prev.x) * (next.y - prev.y) - (curr.y - prev.y) * (next.x - prev.x);

            // If cross product is not near zero, it's a turn/corner, so we must keep the point
            if (std::abs(cross) > 1.0f) {
                guidePathPoints.push_back(curr);
            }
        }
        guidePathPoints.push_back(rawPath.back());
    }
    else {
        guidePathPoints = rawPath;
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
    std::vector<SimpleEnemy>& enemy,
    const ItemsManager& items,
    const std::vector<std::unique_ptr<Room>>& rooms)
{
    bool key1 = AEInputCheckCurr(AEVK_1) != 0;
    bool key2 = AEInputCheckCurr(AEVK_2) != 0;
    bool key3 = AEInputCheckCurr(AEVK_3) != 0;

    if (key1 && !prevKey1)
    {
        // Require POWER_UP item to use Speed Boost
        if (g_Inventory.ConsumeItem(ItemType::POWER_UP))
        {
            AudioSystem::Play("Speed");
            ActivateSpeedBoost(player);
        }
    }

    if (key2 && !prevKey2)
    {
        // Require SLOW_ENEMY item to use Stun
        if (g_Inventory.ConsumeItem(ItemType::SLOW_ENEMY))
        {
            AudioSystem::Play("Stun");
            ActivateStun(enemy);
        }
    }

    if (key3 && !prevKey3)
    {
        // Require POINT item to use Guide Path
        if (g_Inventory.ConsumeItem(ItemType::POINT))
        {
            AudioSystem::Play("Path");
            ActivateGuide(player, items, rooms);
        }
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