// items.cpp
#include "items.h"
#include "Inventory.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>

// Item constructor - NOW 4 TYPES
Item::Item(float posX, float posY, ItemType itemType)
	: x(posX), y(posY), type(itemType), collected(false),
	visualRadius(0.20f),        // Small visual size (looks like a dot)
	collisionRadius(30.0f),      // Large pickup range (easy to collect)
	lifetime(-1.0f), active(true),
	color{ 0.0f, 0.0f, 0.0f, 1.0f } {

	// Set color based on item type - NOW 4 COLORS
	switch (type) {
	case ItemType::POINT:       // RED
		color[0] = 1.0f;   // R = 100%
		color[1] = 0.0f;   // G = 0%
		color[2] = 0.0f;   // B = 0%
		color[3] = 1.0f;   // A = 100%
		break;
	case ItemType::POWER_UP:    // BLUE
		color[0] = 0.0f;   // R = 0%
		color[1] = 0.0f;   // G = 0%
		color[2] = 1.0f;   // B = 100%
		color[3] = 1.0f;   // A = 100%
		break;
	case ItemType::SLOW_ENEMY:  // PURPLE
		color[0] = 1.0f;   // R = 100%
		color[1] = 0.0f;   // G = 0%
		color[2] = 1.0f;   // B = 100%
		color[3] = 1.0f;   // A = 100%
		break;
	case ItemType::KEY:         // YELLOW
		color[0] = 1.0f;   // R = 100%
		color[1] = 1.0f;   // G = 100%
		color[2] = 0.0f;   // B = 0%
		color[3] = 1.0f;   // A = 100%
		break;
	default:
		// Fallback: keep the default initialized color
		break;
	}
}

// Check if player collected this item
bool Item::CheckCollection(float playerX, float playerY) const {
	if (collected || !active) return false;

	float dx = playerX - x;
	float dy = playerY - y;
	float distanceSquared = dx * dx + dy * dy;

	// Use collisionRadius for collection check (not visualRadius)
	return distanceSquared <= (collisionRadius * collisionRadius);
}

// Draw the item
void Item::Draw(AEGfxVertexList* pMesh) const {
	if (collected || !active) return;

	// Use world coordinates
	float drawX = x;
	float drawY = y;

	AEMtx33 scale, trans, transform;

	// Use visualRadius for drawing (keeps items small on screen)
	// Multiply by 2.0f to convert radius to diameter for scaling
	AEMtx33Scale(&scale, visualRadius * 2.0f, visualRadius * 2.0f);
	AEMtx33Trans(&trans, drawX, drawY);
	AEMtx33Concat(&transform, &trans, &scale);

	// Use COLOR mode
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToMultiply(color[0], color[1], color[2], color[3]);
	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);
}

// ItemsManager implementation
ItemsManager::ItemsManager() : tileSize(48), pItemMesh(nullptr) {
	std::srand(static_cast<unsigned>(std::time(nullptr)));
}

void ItemsManager::InitializeGraphics() {
	if (!pItemMesh) {
		CreateItemMesh();
	}
}

// Destructor
ItemsManager::~ItemsManager() {
	// 1. Explicitly clear the vector to call destructors on all 'Item' objects
	items.clear();

	// 2. Force the vector to release its internal capacity
	items.shrink_to_fit();

	// 3. Free the GPU mesh
	if (pItemMesh) {
		AEGfxMeshFree(pItemMesh);
		pItemMesh = nullptr;
	}
}

// Create item mesh
void ItemsManager::CreateItemMesh() {
	// Create a SIMPLE square mesh for items
	AEGfxMeshStart();

	// Simple 1x1 square with WHITE vertices
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
	);
	AEGfxTriAdd(
		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
	);

	pItemMesh = AEGfxMeshEnd();
}

// Initialize the items system
void ItemsManager::Initialize(int gridWidth, int gridHeight,
	const std::vector<std::vector<int>>& maze, float tileSize) {
	this->tileSize = tileSize;
	items.clear();
}

// Spawn a specific item
void ItemsManager::SpawnItem(float x, float y, ItemType type) {
	// Ensure mesh is created before spawning
	if (!pItemMesh) {
		CreateItemMesh();
	}

	items.emplace_back(x, y, type);

	// Debug output
	const char* typeNames[] = { "POINT (RED)", "POWER_UP (BLUE)", "SLOW_ENEMY (PURPLE)", "KEY (YELLOW)" };
	std::cout << "Spawned " << typeNames[static_cast<int>(type)]
		<< " at position (" << x << ", " << y << ")\n";
}

// Spawn a key in a random non-start, non-boss room
void ItemsManager::SpawnKey() {
	// This will be called from Level_Init after rooms are generated
	// The actual spawning will happen in Level_Init
}

// Helper function to find empty tile
bool ItemsManager::IsTileEmpty(int x, int y, const std::vector<std::vector<int>>& maze) const {
	// Check bounds
	if (y < 0 || y >= maze.size() || x < 0 || x >= maze[0].size()) {
		return false;
	}

	// Check if it's a wall
	if (maze[y][x] == 1) {
		return false;
	}

	// Check if there's already an item at this position
	for (const auto& item : items) {
		if (static_cast<int>(item.x) == x && static_cast<int>(item.y) == y && !item.collected) {
			return false;
		}
	}

	return true;
}

// Spawn items randomly in the maze
void ItemsManager::SpawnRandomItems(int count, const std::vector<std::vector<int>>& maze) {
	int spawned = 0;
	int maxAttempts = 1000;

	while (spawned < count && maxAttempts-- > 0) {
		int x = std::rand() % maze[0].size();
		int y = std::rand() % maze.size();

		if (IsTileEmpty(x, y, maze)) {
			// Random item type (0-2 for 3 types - exclude KEY from random spawn)
			int randomType = std::rand() % 3;
			ItemType type = static_cast<ItemType>(randomType);

			SpawnItem(static_cast<float>(x), static_cast<float>(y), type);
			spawned++;
		}
	}

	std::cout << "SpawnRandomItems: " << spawned << " items spawned\n";
}

// Update items (check collection, handle lifetime)
void ItemsManager::Update(float playerX, float playerY, float deltaTime) {
	for (auto& item : items) {
		if (!item.collected && item.active) {
			// Check collection
			if (item.CheckCollection(playerX, playerY)) {
				item.collected = true;
				item.active = false;

				// Add to inventory
				g_Inventory.AddItem(item.type);

				// Item effects (just for feedback)
				switch (item.type) {
				case ItemType::POINT:
					printf("+10 Points! (Added to inventory)\n");
					break;
				case ItemType::POWER_UP:
					printf("Power-up collected! (Added to inventory)\n");
					break;
				case ItemType::SLOW_ENEMY:
					printf("Enemies slowed! (Added to inventory)\n");
					break;
				case ItemType::KEY:
					printf("KEY collected! You can now exit the dungeon!\n");
					break;
				}

				std::cout << "Item collected at (" << item.x << ", " << item.y << ")\n";
			}
		}
	}
}

// Draw all items
void ItemsManager::Draw() const {
	// Don't draw if mesh isn't created yet
	if (!pItemMesh) return;

	for (const auto& item : items) {
		item.Draw(pItemMesh);
	}
}

// Get collected items count
int ItemsManager::GetCollectedCount() const {
	return static_cast<int>(std::count_if(items.begin(), items.end(),
		[](const Item& item) { return item.collected; }));
}

// Get total items count
int ItemsManager::GetTotalCount() const {
	return static_cast<int>(items.size());
}

// Reset all items
void ItemsManager::Reset() {
	for (auto& item : items) {
		item.collected = false;
		item.active = true;
	}
	std::cout << "All items reset\n";
}

// Check if all items are collected
bool ItemsManager::AllItemsCollected() const {
	return GetCollectedCount() == GetTotalCount() && GetTotalCount() > 0;
}