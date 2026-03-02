#include "jogo.h" 
#include "Items.h"     
#include <iostream>
#include <cmath>       

// Constructor initializes position and settings
Character::Character(int startX, int startY, float tile)
	: gridX(startX), gridY(startY), tileSize(tile), pMesh(nullptr), moveTimer(0.0f)
{
	// Centers the world position within the grid tile
	worldX = (static_cast<float>(gridX) * tileSize) + (tileSize * 0.5f);
	worldY = (static_cast<float>(gridY) * tileSize) + (tileSize * 0.5f);
	moveSpeed = 400.0f;
	isMoving = false;
}

// Destructor calls unload
Character::~Character()
{
	Unload();
}

// Creates the player graphic
void Character::Load()
{
	if (pMesh != nullptr)
	{
		return;
	}

	AEGfxMeshStart();
	// Create a square mesh with center origin
	AEGfxTriAdd(-0.5f, -0.5f, 0xFF00FFFF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFF00FFFF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFF00FFFF, 0.0f, 0.0f);
	AEGfxTriAdd(0.5f, -0.5f, 0xFF00FFFF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFF00FFFF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFF00FFFF, 0.0f, 0.0f);
	pMesh = AEGfxMeshEnd();
}

// Frees the player graphic
void Character::Unload()
{
	if (pMesh)
	{
		AEGfxMeshFree(pMesh);
		pMesh = nullptr;
	}
}

// Update loop handling input and collision
void Character::Update(const std::vector<std::unique_ptr<Room>>& rooms)
{
	float dt = static_cast<float>(AEFrameRateControllerGetFrameTime());
	float dirX = 0.0f;
	float dirY = 0.0f;

	// Check input keys for movement
	if (AEInputCheckCurr(AEVK_W) || AEInputCheckCurr(AEVK_UP))   dirY += 1.0f;
	if (AEInputCheckCurr(AEVK_S) || AEInputCheckCurr(AEVK_DOWN)) dirY -= 1.0f;
	if (AEInputCheckCurr(AEVK_A) || AEInputCheckCurr(AEVK_LEFT)) dirX -= 1.0f;
	if (AEInputCheckCurr(AEVK_D) || AEInputCheckCurr(AEVK_RIGHT)) dirX += 1.0f;

	if (dirX != 0.0f || dirY != 0.0f)
	{
		isMoving = true;
		// Normalize vector to prevent fast diagonal movement
		float length = sqrtf(dirX * dirX + dirY * dirY);
		if (length > 0.0f)
		{
			dirX /= length;
			dirY /= length;
		}

		// SLIDING COLLISION LOGIC
		// Calculate and apply X and Y movement separately
		// Allow the player to slide along walls instead of getting stuck

		// Try moving along X-Axis
		float nextWorldX = worldX + (dirX * moveSpeed * dt);
		if (IsPositionWalkable(nextWorldX, worldY, rooms))
		{
			worldX = nextWorldX;
		}

		// Try moving along Y-Axis
		// Use the potentially updated worldX to ensure no slide into a corner wall
		float nextWorldY = worldY + (dirY * moveSpeed * dt);
		if (IsPositionWalkable(worldX, nextWorldY, rooms))
		{
			worldY = nextWorldY;
		}

		// Update grid coordinates based on final position
		gridX = static_cast<int>(floorf(worldX / tileSize));
		gridY = static_cast<int>(floorf(worldY / tileSize));

		moveTimer += dt * 12.0f;

		// Update Fog of War based on world position
		for (const auto& room : rooms)
		{
			if (worldX >= (float)room->rect.left && worldX <= (float)room->rect.right &&
				worldY >= (float)room->rect.bottom && worldY <= (float)room->rect.top)
			{
				room->isDiscovered = true;
			}
		}
	}
	else
	{
		isMoving = false;
		moveTimer = 0.0f;
	}
}

// Renders the character mesh
void Character::Draw()
{
	if (!pMesh)
	{
		return;
	}

	AEMtx33 scale, rot, trans, transform;

	// Bobbing effect when moving
	float bobOffset = isMoving ? fabsf(sinf(moveTimer)) * 8.0f : 0.0f;
	float tiltAngle = isMoving ? sinf(moveTimer) * 0.1f : 0.0f;

	AEMtx33Scale(&scale, tileSize * 0.4f, tileSize * 0.4f);
	AEMtx33Rot(&rot, tiltAngle);
	AEMtx33Trans(&trans, worldX, worldY + bobOffset);

	AEMtx33Concat(&transform, &rot, &scale);
	AEMtx33Concat(&transform, &trans, &transform);

	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);
}

// Teleport function
void Character::SetPosition(int x, int y)
{
	gridX = x;
	gridY = y;
	worldX = (static_cast<float>(gridX) * tileSize) + (tileSize * 0.5f);
	worldY = (static_cast<float>(gridY) * tileSize) + (tileSize * 0.5f);
}

// Check for item collection automatically (no key press needed)
void Character::CheckItemCollection(ItemsManager& itemsManager)
{
	// Call Update on items manager with player's current position
	// This will check collision for all items
	itemsManager.Update(worldX, worldY, 0.0f);
}

// Checks collision against the specific tile map of the room
bool Character::IsPositionWalkable(float x, float y, const std::vector<std::unique_ptr<Room>>& rooms)
{
	for (const auto& room : rooms)
	{
		// Check if inside Room Bounding Box
		if (x >= (float)room->rect.left && x <= (float)room->rect.right &&
			y >= (float)room->rect.bottom && y <= (float)room->rect.top)
		{
			// Calculate which tile grid index the player is on
			// Local Position equals World Pos minus Room Top Left
			float localX = x - (float)room->rect.left;
			float localY = (float)room->rect.top - y;

			int tileX = static_cast<int>(localX / room->tileSize);
			int tileY = static_cast<int>(localY / room->tileSize);

			// Check the specific tile value where 0 is Floor and 1 is Wall
			if (room->GetTile(tileX, tileY) == 0)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	// Void outside all rooms is treated as a wall
	return false;
}