#include "Enemy.h"
#include <math.h> // For sqrtf

SimpleEnemy::SimpleEnemy()
    : worldX(0), worldY(0), startX(0), startY(0),
    speed(200.0f), detectionRange(500.0f), giveUpRange(150.0f),
    currentState(EnemyState::IDLE), pMesh(nullptr), stunTimer(0.0f)
{
}

SimpleEnemy::~SimpleEnemy()
{
    Unload();
}

void SimpleEnemy::Load()
{
    if (pMesh) return;
    AEGfxMeshStart();
    // Red Square Mesh
    AEGfxTriAdd(-0.5f, -0.5f, 0xFF0000FF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFF0000FF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFF0000FF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFF0000FF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFF0000FF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFF0000FF, 0.0f, 0.0f);
    pMesh = AEGfxMeshEnd();
}

void SimpleEnemy::Unload()
{
    if (pMesh) {
        AEGfxMeshFree(pMesh);
        pMesh = nullptr;
    }
}

void SimpleEnemy::SetPosition(float x, float y)
{
    worldX = x;
    worldY = y;
    // Memorize the home location
    startX = x;
    startY = y;
}

void SimpleEnemy::Stun(float duration)
{
    if (duration > stunTimer)
    {
        stunTimer = duration;
    }
}

void SimpleEnemy::Update(float playerX, float playerY, float dt, const std::vector<std::unique_ptr<Room>>& rooms)
{
    // stun enemy
    if (stunTimer > 0.0f)
    {
        stunTimer -= dt;
        if (stunTimer < 0.0f)
        {
            stunTimer = 0.0f;
        }
        return;
    }

    // Calculate distance to Player
    float dx = playerX - worldX;
    float dy = playerY - worldY;
    float distToPlayer = sqrtf(dx * dx + dy * dy);

    // Calculate distance to Home (Start)
    float hx = startX - worldX;
    float hy = startY - worldY;
    float distToHome = sqrtf(hx * hx + hy * hy);

    // --- STATE MACHINE LOGIC ---
    switch (currentState)
    {
        // 1. IDLE: Wait for player
    case EnemyState::IDLE:
        if (distToPlayer < detectionRange)
        {
            currentState = EnemyState::CHASE;
        }
        break;

        // 2. CHASE: Move towards player
    case EnemyState::CHASE:
        // logic: If player runs too far, switch to RETURN
        if (distToPlayer > giveUpRange)
        {
            currentState = EnemyState::RETURN;
        }
        else
        {
            // Move towards player
            if (distToPlayer > 0.01f) // Avoid division by zero
            {
                float dirX = dx / distToPlayer;
                float dirY = dy / distToPlayer;

                // Calculate next position
                float nextX = worldX + (dirX * speed * dt);
                float nextY = worldY + (dirY * speed * dt);

                // Apply Collision (Slide)
                if (IsPosValid(nextX, worldY, rooms)) worldX = nextX;
                if (IsPosValid(worldX, nextY, rooms)) worldY = nextY;
            }
        }
        break;

        // 3. RETURN: Move back to start position
    case EnemyState::RETURN:
        // Logic: If player comes close again, resume CHASE
        if (distToPlayer < detectionRange)
        {
            currentState = EnemyState::CHASE;
        }
        // Logic: If we reached home, switch to IDLE
        else if (distToHome < 10.0f)
        {
            worldX = startX;
            worldY = startY;
            currentState = EnemyState::IDLE;
        }
        else
        {
            // Move towards Start Position
            float dirX = hx / distToHome;
            float dirY = hy / distToHome;

            float nextX = worldX + (dirX * speed * dt);
            float nextY = worldY + (dirY * speed * dt);

            // Apply Collision (Slide)
            // Note: If the path home is blocked by a wall, it might get stuck.
            // But since it walked out, it can usually walk back.
            if (IsPosValid(nextX, worldY, rooms)) worldX = nextX;
            if (IsPosValid(worldX, nextY, rooms)) worldY = nextY;
        }
        break;
    }
}

void SimpleEnemy::Draw()
{
    if (!pMesh) return;

    // Change color based on state for visual feedback
    if (IsStunned())
        AEGfxSetColorToMultiply(0.0f, 0.5f, 1.0f, 1.0f);
    else if (currentState == EnemyState::IDLE)
        AEGfxSetColorToMultiply(0.5f, 0.5f, 0.5f, 1.0f); // Grey (Sleeping)
    else if (currentState == EnemyState::CHASE)
        AEGfxSetColorToMultiply(1.0f, 0.2f, 0.2f, 1.0f); // Bright Red (Angry)
    else if (currentState == EnemyState::RETURN)
        AEGfxSetColorToMultiply(1.0f, 1.0f, 0.0f, 1.0f); // Yellow (Confused/Returning)

    AEMtx33 scale, trans, transform;
    AEMtx33Scale(&scale, 25.0f, 25.0f);
    AEMtx33Trans(&trans, worldX, worldY);
    AEMtx33Concat(&transform, &trans, &scale);

    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);
}

bool SimpleEnemy::IsPosValid(float x, float y, const std::vector<std::unique_ptr<Room>>& rooms)
{
    for (const auto& room : rooms)
    {
        if (x >= room->rect.left && x <= room->rect.right &&
            y >= room->rect.bottom && y <= room->rect.top)
        {
            return true;
        }
    }
    return false;
}
