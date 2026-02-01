#ifndef PLAYER_H
#define PLAYER_H

#include "AEEngine.h"

// The player class encapsulates movement, rendering, and spatial data
class Player {
public:
    AEVec2 pos;             // Current world-space coordinates
    float speed;            // Movement velocity in pixels per second
    AEGfxVertexList* pMesh; // Pointer to the triangle geometric data

    Player();

    // Configures the player mesh and sets the starting position
    void Init(AEVec2 startPos);

    // Processes input to move the player around the world
    void Update(float dt);

    // Renders the player as a yellow triangle at their current position
    void Draw();

    // Releases the player's GPU resources
    void Unload();
};

#endif