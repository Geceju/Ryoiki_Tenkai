#ifndef JOGO_H
#define JOGO_H

#include "AEEngine.h"
#include <vector>
#include <memory>
#include "Room.h" 
#include "ability.h"

class ItemsManager;
class SimpleEnemy;

// Handles player movement collision and rendering
class Character
{
public:
	// Constructor taking grid coordinates and tile size
	Character(int startX, int startY, float tile);

	// Destructor to clean up resources
	~Character();

	// Create the player mesh
	void Load();

	// Release the player mesh
	void Unload();

	// Core loop functions
	// Update logic accepts room list for tile collision checking
	void Update(const std::vector<std::unique_ptr<Room>>& rooms);

	// Renders the player mesh
	void Draw();

	// Set position directly using grid coordinates
	void SetPosition(int x, int y);

	// Getters for position
	int GetGridX() const { return gridX; }
	int GetGridY() const { return gridY; }
	float GetWorldX() const { return worldX; }
	float GetWorldY() const { return worldY; }

	// Logic for item interaction
	void CheckItemCollection(ItemsManager& itemsManager);

	// Abilities
	float GetMoveSpeed() const { return moveSpeed; }
	void SetMoveSpeed(float speed) { moveSpeed = speed; }

	void UpdateAbilities(float dt, std::vector<SimpleEnemy>& enemy, const ItemsManager& items, const std::vector<std::unique_ptr<Room>>& rooms);
	void DrawAbilities() const;
	void TriggerStun(std::vector<SimpleEnemy>& enemy);

	// NEW VISION VARIABLES 
	float facingAngle;                 // Tracks which way the player is aiming
	AEGfxVertexList* pVisionMesh;      // The darkness overlay mesh
	float visionMultiplier;

	void LoadVisionMesh();             // Generates the shadow geometry
	void DrawVisionOverlay();          // Renders the shadow over the world

private:
	// Checks specific tile value at coordinates
	// Return true only if the tile is 0 meaning Floor
	bool IsPositionWalkable(float x, float y, const std::vector<std::unique_ptr<Room>>& rooms);

	// Grid tracking
	int gridX;
	int gridY;
	float tileSize;

	// Animation and Movement variables
	float worldX;
	float worldY;
	float moveSpeed;
	float moveTimer;
	bool isMoving;

	// Visual mesh
	AEGfxVertexList* pMesh;

	// Abilities
	PlayerAbilities abilities;
};

#endif