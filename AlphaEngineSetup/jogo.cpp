//author : Felicia
#include "Jogo.h" 
#include "Items.h"   
#include "Enemy.h"
#include "AudioSystem.h"
#include <iostream>
#include <cmath>       

// Constructor initializes position and settings
Character::Character(int startX, int startY, float tile)
	: gridX(startX), gridY(startY), tileSize(tile), pMesh(nullptr), pTexture(nullptr), moveTimer(0.0f)
{
	// Centers the world position within the grid tile
	worldX = (static_cast<float>(gridX) * tileSize) + (tileSize * 0.5f);
	worldY = (static_cast<float>(gridY) * tileSize) + (tileSize * 0.5f);
	moveSpeed = 200.0f;
	stepTimer = 0.0f;
	isMoving = false;
	// --- NEW ---
	facingAngle = 0.0f;
	pVisionMesh = nullptr;
	visionMultiplier = 1.0f; // <-- ADD THIS
}

// Destructor calls unload
Character::~Character()
{
	Unload();
}

// Creates the player graphic
void Character::Load()
{
	if (pMesh != nullptr) return;

	// 1. Load the texture (Make sure the path matches your folder exactly!)
	pTexture = AEGfxTextureLoad("Assets/jogo.png");

	AEGfxMeshStart();
	// 2. PURE WHITE MESH (0xFFFFFFFF) so the texture's true colors show up
	AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	pMesh = AEGfxMeshEnd();

	// Load abilities and vision mesh
	abilities.Load();
	LoadVisionMesh();
}

// Frees the player graphic
void Character::Unload()
{
	abilities.Unload();

	if (pMesh)
	{
		AEGfxMeshFree(pMesh);
		pMesh = nullptr;
	}

	if (pTexture)
	{
		AEGfxTextureUnload(pTexture);
		pTexture = nullptr;
	}

	// Free vision mesh if it exists
	if (pVisionMesh) {
		AEGfxMeshFree(pVisionMesh);
		pVisionMesh = nullptr;
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
		facingAngle = atan2f(dirY, dirX);
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

		// If Noclip is ON, or the next X position is a floor tile, move X!
		if (isNoClip || IsPositionWalkable(nextWorldX, worldY, rooms))
		{
			worldX = nextWorldX;
		}

		// Try moving along Y-Axis
		float nextWorldY = worldY + (dirY * moveSpeed * dt);

		// If Noclip is ON, or the next Y position is a floor tile, move Y!
		if (isNoClip || IsPositionWalkable(worldX, nextWorldY, rooms))
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

	if (isMoving) {
		// Increment timer by delta time
		stepTimer += dt;

		// Trigger a single "thud/step" sound every 0.35 seconds
		if (stepTimer >= 0.35f) {
			AudioSystem::Play("Footsteps");
			stepTimer = 0.0f; // Reset the timer for the next step
		}
	}
	else {
		// Reset timer when standing still so the next step starts instantly when moving
		stepTimer = 0.35f;
	}
}

// Abilities
void Character::UpdateAbilities(float dt, std::vector<SimpleEnemy>& enemy, const ItemsManager& items, const std::vector<std::unique_ptr<Room>>& rooms)
{
	abilities.Update(dt, *this, enemy, items, rooms);
}

// Renders the character mesh
void Character::Draw()
{
	if (!pMesh) return;

	AEMtx33 scale, rot, trans, transform;

	// Bobbing effect when moving
	float bobOffset = isMoving ? fabsf(sinf(moveTimer)) * 8.0f : 0.0f;
	float tiltAngle = isMoving ? sinf(moveTimer) * 0.1f : 0.0f;

	// Scaled up to 0.8f so the character isn't tiny!
	AEMtx33Scale(&scale, tileSize * 2.0f, tileSize * 2.0f);
	AEMtx33Rot(&rot, tiltAngle);
	AEMtx33Trans(&trans, worldX, worldY + bobOffset);

	AEMtx33Concat(&transform, &rot, &scale);
	AEMtx33Concat(&transform, &trans, &transform);

	// 1. ENABLE BLENDING for the transparent background
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);

	// 2. SET RENDER MODE TO TEXTURE
	AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
	AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
	AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f); // Safety reset

	// 3. DRAW THE TEXTURE
	AEGfxTextureSet(pTexture, 0.0f, 0.0f);
	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);

	// 4. CLEAN UP FOR THE REST OF THE GAME
	AEGfxTextureSet(nullptr, 0.0f, 0.0f);
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetBlendMode(AE_GFX_BM_NONE);
}

// Draw Abilities
void Character::DrawAbilities() const
{
	abilities.DrawGuide();
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
void Character::LoadVisionMesh()
{
	if (pVisionMesh) return;

	AEGfxMeshStart();
	int segments = 64; // High segment count for a smooth circle
	float step = (2.0f * 3.14159265f) / segments;

	// --- TWEAK THESE TO CHANGE YOUR FLASHLIGHT ---
	float ambientRadius = 250.0f * visionMultiplier;
	float darkRadius = 350.0f * visionMultiplier;
	// -----------------------------------------------
	float farRadius = 4000.0f;     // Massive outer bounds to cover the entire screen
	float coneHalfAngle = (60.0f / 2.0f) * (3.14159265f / 180.0f); // 60-degree vision cone

	for (int i = 0; i < segments; i++)
	{
		float a1 = i * step;
		float a2 = (i + 1) * step;

		// Normalize angles between -PI and PI to easily check our cone
		float normA1 = a1; if (normA1 > 3.14159265f) normA1 -= 2.0f * 3.14159265f;
		float normA2 = a2; if (normA2 > 3.14159265f) normA2 -= 2.0f * 3.14159265f;

		// Is this segment inside the flashlight cone?
		bool cone1 = (fabsf(normA1) <= coneHalfAngle);
		bool cone2 = (fabsf(normA2) <= coneHalfAngle);

		// Alpha values: 0 = Bright/Transparent, 220 = Dim, 255 = Pitch Black
		unsigned int alphaCenter1 = cone1 ? 0 : 220;
		unsigned int alphaCenter2 = cone2 ? 0 : 220;
		unsigned int alphaAmb1 = cone1 ? 0 : 220;
		unsigned int alphaAmb2 = cone2 ? 0 : 220;
		unsigned int alphaDark = 255;
		unsigned int alphaFar = 255;

		// Convert Alpha to AARRGGBB format (Colors remain 000000 Black)
		unsigned int cCenter1 = (alphaCenter1 << 24);
		unsigned int cCenter2 = (alphaCenter2 << 24);
		unsigned int cAmb1 = (alphaAmb1 << 24);
		unsigned int cAmb2 = (alphaAmb2 << 24);
		unsigned int cDark = (alphaDark << 24);
		unsigned int cFar = (alphaFar << 24);

		// Calculate Ring Coordinates
		float cx = 0.0f, cy = 0.0f;
		float a1x = cosf(a1) * ambientRadius, a1y = sinf(a1) * ambientRadius;
		float a2x = cosf(a2) * ambientRadius, a2y = sinf(a2) * ambientRadius;
		float d1x = cosf(a1) * darkRadius, d1y = sinf(a1) * darkRadius;
		float d2x = cosf(a2) * darkRadius, d2y = sinf(a2) * darkRadius;
		float f1x = cosf(a1) * farRadius, f1y = sinf(a1) * farRadius;
		float f2x = cosf(a2) * farRadius, f2y = sinf(a2) * farRadius;

		// 1. Center to Ambient (Inner Triangle)
		AEGfxTriAdd(cx, cy, cCenter1, 0, 0, a1x, a1y, cAmb1, 0, 0, a2x, a2y, cAmb2, 0, 0);
		// 2. Ambient to Dark (Mid Quad -> Fades out the flashlight smoothly)
		AEGfxTriAdd(a1x, a1y, cAmb1, 0, 0, d1x, d1y, cDark, 0, 0, a2x, a2y, cAmb2, 0, 0);
		AEGfxTriAdd(a2x, a2y, cAmb2, 0, 0, d1x, d1y, cDark, 0, 0, d2x, d2y, cDark, 0, 0);
		// 3. Dark to Far (Outer Quad -> Infinite Pitch Black)
		AEGfxTriAdd(d1x, d1y, cDark, 0, 0, f1x, f1y, cFar, 0, 0, d2x, d2y, cDark, 0, 0);
		AEGfxTriAdd(d2x, d2y, cDark, 0, 0, f1x, f1y, cFar, 0, 0, f2x, f2y, cFar, 0, 0);
	}
	pVisionMesh = AEGfxMeshEnd();
}

void Character::DrawVisionOverlay()
{
	if (!pVisionMesh) return;

	AEMtx33 scale, rot, trans, transform;
	AEMtx33Scale(&scale, 1.0f, 1.0f);
	AEMtx33Rot(&rot, facingAngle); // Rotates the flashlight cone!
	AEMtx33Trans(&trans, worldX, worldY); // Locks it to the player

	AEMtx33Concat(&transform, &rot, &scale);
	AEMtx33Concat(&transform, &trans, &transform);

	// Alpha blending MUST be on for the darkness gradient to work!
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(pVisionMesh, AE_GFX_MDM_TRIANGLES);
	AEGfxSetBlendMode(AE_GFX_BM_NONE); // Turn off when done
}

void Character::TriggerStun(std::vector<SimpleEnemy>& enemy) {
	Character::abilities.ActivateStun(enemy);
}