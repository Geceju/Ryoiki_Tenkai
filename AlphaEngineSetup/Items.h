// items.h - Add InitializeGraphics method
#pragma once
#include "AEEngine.h"
#include <vector>

// Item types enumeration - ONLY 3 TYPES
enum class ItemType {
	POINT,      // Red - Basic points
	POWER_UP,   // Blue - Temporary boost
	SLOW_ENEMY  // Purple - Slows enemy down
};

// Item structure
struct Item {
	float x, y;             // World coordinates (in tile units)
	ItemType type;          // Type of item
	bool collected;         // Collection status
	float visualRadius;     // Size for rendering (small)
	float collisionRadius;  // Size for collision detection (large)
	f32 color[4];           // RGBA color for rendering
	float lifetime;         // How long it exists (optional)
	bool active;            // If still active in game

	// Constructor
	Item(float posX, float posY, ItemType itemType);

	// Check if player collected this item
	bool CheckCollection(float playerX, float playerY) const;

	// Draw the item
	void Draw(AEGfxVertexList* pMesh) const;
};

// Items manager class
class ItemsManager {
private:
	std::vector<Item> items;
	AEGfxVertexList* pItemMesh;
	float tileSize;

public:
	// Constructor & Destructor
	ItemsManager();
	~ItemsManager();

	// Initialize the items system
	void Initialize(int gridWidth, int gridHeight, const std::vector<std::vector<int>>& maze, float tileSize = 48.0f);

	// Initialize graphics (call this AFTER engine is initialized)
	void InitializeGraphics(); // ADD THIS

	// Spawn a specific item
	void SpawnItem(float x, float y, ItemType type);

	// Spawn items randomly in the maze
	void SpawnRandomItems(int count, const std::vector<std::vector<int>>& maze);

	// Update items (check collection, handle lifetime)
	void Update(float playerX, float playerY, float deltaTime);

	// Draw all items
	void Draw() const;

	// Get collected items count
	int GetCollectedCount() const;

	// Get total items count
	int GetTotalCount() const;

	// Reset all items
	void Reset();

	// Get reference to items (for external processing)
	std::vector<Item>& GetItems() { return items; }
	const std::vector<Item>& GetItems() const { return items; }

	// Get item mesh
	AEGfxVertexList* GetItemMesh() const { return pItemMesh; }

	// Check if all items are collected
	bool AllItemsCollected() const;

private:
	// Helper function to find empty tile
	bool IsTileEmpty(int x, int y, const std::vector<std::vector<int>>& maze) const;

	// Create item mesh
	void CreateItemMesh();
};