//author : Felicia
#include "Ability.h"
#include "Jogo.h"
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
    guideOrbDistFromItem(0.0f),
    guidePathTotalLength(0.0f),
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

    // Create a 16-sided circle
    const int segments = 16;

    for (int i = 0; i < segments; ++i)
    {
        float theta1 = 2.0f * PI * float(i) / float(segments);
        float theta2 = 2.0f * PI * float(i + 1) / float(segments);

        // Calculate outer points (radius of 0.5f keeps it the same base scale as before)
        float x1 = 0.5f * std::cos(theta1);
        float y1 = 0.5f * std::sin(theta1);
        float x2 = 0.5f * std::cos(theta2);
        float y2 = 0.5f * std::sin(theta2);

        // Draw a triangle from the center to the edge
        AEGfxTriAdd(
            0.0f, 0.0f, 0xFFFFFFFF, 0.5f, 0.5f, // Center point
            x1, y1, 0xFFFFFFFF, 0.0f, 0.0f,     // Edge point 1
            x2, y2, 0xFFFFFFFF, 0.0f, 0.0f      // Edge point 2
        );
    }

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

// world coordinates -> global tile grid
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

// tile grid -> global world coordinates
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
    guidePathTotalLength = 0.0f;
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

    // BFS outward from player to find the TRUE shortest walking path
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

    // Path Simplifier (Creates straight lines by removing jagged steps)
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

    // --- CALCULATE TOTAL LENGTH EVERY TIME WE REBUILD THE PATH ---
    guidePathTotalLength = 0.0f;
    for (size_t i = 0; i + 1 < guidePathPoints.size(); ++i) {
        float dx = guidePathPoints[i + 1].x - guidePathPoints[i].x;
        float dy = guidePathPoints[i + 1].y - guidePathPoints[i].y;
        guidePathTotalLength += std::sqrt(dx * dx + dy * dy);
    }
}

void PlayerAbilities::ActivateGuide(const Character& player,
    const ItemsManager& items,
    const std::vector<std::unique_ptr<Room>>& rooms)
{
    guideTimer = 20.0f;
    pathRefreshTimer = 0.0f;
    RebuildGuidePath(player, items, rooms);

    // Start the orb at the player's position (max distance from item)
    guideOrbDistFromItem = guidePathTotalLength;
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

        // Move the orb TOWARDS the item (decreasing the distance)
        guideOrbDistFromItem -= 180.0f * dt;

        // If the orb reaches the item, wait a moment (-150.0f), then respawn at player
        if (guideOrbDistFromItem < -150.0f) {
            guideOrbDistFromItem = guidePathTotalLength;
        }

        if (pathRefreshTimer <= 0.0f)
        {
            pathRefreshTimer = 0.25f;
            RebuildGuidePath(player, items, rooms);

            // Safety check: if the player walked backwards, don't let the orb spawn behind them
            if (guideOrbDistFromItem > guidePathTotalLength) {
                guideOrbDistFromItem = guidePathTotalLength;
            }
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
    if (!pGuideMesh || guidePathPoints.size() < 2 || guidePathTotalLength <= 0.001f)
    {
        return;
    }

    // We need segment lengths for interpolation
    std::vector<float> segmentLengths;
    for (size_t i = 0; i + 1 < guidePathPoints.size(); ++i)
    {
        float dx = guidePathPoints[i + 1].x - guidePathPoints[i].x;
        float dy = guidePathPoints[i + 1].y - guidePathPoints[i].y;
        segmentLengths.push_back(std::sqrt(dx * dx + dy * dy));
    }

    // Convert "distance from item" to "distance from player" for drawing
    float currentDist = guidePathTotalLength - guideOrbDistFromItem;

    // --- HELPER: Finds the exact X/Y coordinate at any given distance along the path ---
    auto GetPosAtDistance = [&](float dist, AEVec2& outPos) -> bool {
        if (dist < 0.0f || dist > guidePathTotalLength) return false;

        float segmentStart = 0.0f;
        for (size_t i = 0; i < segmentLengths.size(); ++i)
        {
            float segLen = segmentLengths[i];
            if (dist <= segmentStart + segLen)
            {
                float t = (dist - segmentStart) / segLen;
                outPos.x = guidePathPoints[i].x + (guidePathPoints[i + 1].x - guidePathPoints[i].x) * t;
                outPos.y = guidePathPoints[i].y + (guidePathPoints[i + 1].y - guidePathPoints[i].y) * t;
                return true;
            }
            segmentStart += segLen;
        }
        return false;
        };

    // --- HELPER: Draws a YELLOW circle ---
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    auto DrawCircle = [&](float x, float y, float size, float alpha) {
        AEMtx33 scale, trans, finalMtx;
        AEMtx33Scale(&scale, size, size);
        AEMtx33Trans(&trans, x, y);
        AEMtx33Concat(&finalMtx, &trans, &scale);

        // 1.0f Red + 1.0f Green = Yellow
        AEGfxSetColorToMultiply(1.0f, 1.0f, 0.0f, alpha);
        AEGfxSetTransform(finalMtx.m);
        AEGfxMeshDraw(pGuideMesh, AE_GFX_MDM_TRIANGLES);
        };

    // 1. Draw the fading trail FIRST (so it renders behind the main ball)
    int trailCount = 10;
    float trailSpacing = 8.0f; // Spaced out slightly more for the higher speed

    for (int i = 1; i <= trailCount; ++i)
    {
        float trailDist = currentDist - (i * trailSpacing);
        AEVec2 trailPos;

        if (GetPosAtDistance(trailDist, trailPos))
        {
            float fadeFactor = 1.0f - ((float)i / trailCount);
            float trailSize = 12.0f * fadeFactor;
            float trailAlpha = fadeFactor * 0.5f;

            DrawCircle(trailPos.x, trailPos.y, trailSize, trailAlpha);
        }
    }

    // 2. Draw the Main glowing core and magical orbiting sparkles
    AEVec2 centerPos;
    if (GetPosAtDistance(currentDist, centerPos))
    {
        // The bright inner core and soft outer glow
        DrawCircle(centerPos.x, centerPos.y, 12.0f, 1.0f);
        DrawCircle(centerPos.x, centerPos.y, 22.0f, 0.3f);

        // NEW: Draw a cluster of orbiting magical particles
        int numSparkles = 6;
        for (int i = 0; i < numSparkles; ++i)
        {
            // Use the moving distance as a continuous "time" variable to animate the particles
            float timeVar = guideOrbDistFromItem * 0.05f;

            // Alternate rotation direction: even index spins one way, odd index spins the other
            float spinSpeed = (i % 2 == 0) ? 1.2f : -1.8f;
            float angle = (timeVar * spinSpeed) + (i * (6.28318f / numSparkles));

            // Make the orbit radius pulse in and out slightly
            float orbitRadius = 14.0f + std::sin(timeVar * 0.5f + i) * 6.0f;

            float px = centerPos.x + std::cos(angle) * orbitRadius;
            float py = centerPos.y + std::sin(angle) * orbitRadius;

            // Make the individual sparkles pulse in size
            float sparkleSize = 4.0f + std::sin(timeVar + i) * 2.0f;

            // Draw the sparkle!
            DrawCircle(px, py, sparkleSize, 0.9f);
        }
    }
}