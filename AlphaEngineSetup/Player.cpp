#include "Player.h"

Player::Player() : speed(400.0f), pMesh(nullptr)
{
    // We initialize the player's position to the origin by default.
    // This serves as a safety fallback before the Level_Init sets the actual spawn point.
    pos.x = 0;
    pos.y = 0;
}

void Player::Init(AEVec2 startPos)
{
    // The player's world position is set based on the starting room's coordinates.
    pos = startPos;

    // We define a simple triangle mesh to represent the player character.
    // The vertices are defined relative to (0,0) to ensure the triangle rotates and scales 
    // around its own center rather than a distant corner.
    AEGfxMeshStart();
    AEGfxTriAdd(0.0f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
        -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
        0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pMesh = AEGfxMeshEnd();
}

void Player::Update(float dt)
{
    // We process keyboard input to update the player's position in world space.
    // Multiplying the speed by delta time ensures the movement remains smooth and consistent
    // regardless of whether the game is running at 30 or 144 frames per second.
    if (AEInputCheckCurr(AEVK_W))
    {
        pos.y += speed * dt;
    }

    if (AEInputCheckCurr(AEVK_S))
    {
        pos.y -= speed * dt;
    }

    if (AEInputCheckCurr(AEVK_A))
    {
        pos.x -= speed * dt;
    }

    if (AEInputCheckCurr(AEVK_D))
    {
        pos.x += speed * dt;
    }

    // BOUNDARY CLAMPING: We calculate the half-extents of the 1600x900 map.
    // Since the dungeon is centered at (0,0), the limits are +/- 800 and +/- 450.
    float boundaryX = 3200.0f / 2.0f;
    float boundaryY = 1800.0f / 2.0f;

    // We check the player's X position and snap it back if it exceeds the horizontal edges.
    if (pos.x > boundaryX)
    {
        pos.x = boundaryX;
    }
    else if (pos.x < -boundaryX)
    {
        pos.x = -boundaryX;
    }

    // We check the player's Y position and snap it back if it exceeds the vertical edges.
    if (pos.y > boundaryY)
    {
        pos.y = boundaryY;
    }
    else if (pos.y < -boundaryY)
    {
        pos.y = -boundaryY;
    }
}

void Player::Draw()
{
    // We construct a transformation matrix by combining a scale and a translation.
    // This blows up our tiny 1x1 unit triangle to a visible 20x20 size and places it at the player's current position.
    AEMtx33 scale, trans, transform;
    AEMtx33Scale(&scale, 20.0f, 20.0f);
    AEMtx33Trans(&trans, pos.x, pos.y);
    AEMtx33Concat(&transform, &trans, &scale);

    // We apply the transform and set the color to a bright yellow so the player is easily 
    // visible against the darker dungeon floor tiles.
    AEGfxSetTransform(transform.m);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 0.0f, 1.0f);
    AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);
}

void Player::Unload()
{
    // We explicitly release the vertex list from the GPU's memory.
    // This is crucial to prevent memory leaks when restarting the game or switching levels.
    if (pMesh)
    {
        AEGfxMeshFree(pMesh);
        pMesh = nullptr;
    }
}