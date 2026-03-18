#ifndef INVENTORY_H
#define INVENTORY_H

#include "AEEngine.h"
#include "Items.h"
#include <array>

// Inventory slot structure
struct InventorySlot {
	ItemType itemType;
	int count;
	bool isActive;

	InventorySlot() : itemType(ItemType::POINT), count(0), isActive(false) {}
};

class Inventory {
private:
	static constexpr int HOTBAR_SIZE = 4;
	static constexpr float SLOT_SIZE = 64.0f;      // Size of each slot in pixels
	static constexpr float SLOT_SPACING = 8.0f;    // Space between slots
	static constexpr float Y_OFFSET = 50.0f;       // Distance from bottom of screen
	static constexpr float X_OFFSET = 20.0f;       // Distance from left edge

	std::array<InventorySlot, HOTBAR_SIZE> slots;
	int selectedSlot;

	// Visual elements
	AEGfxVertexList* pSlotMesh;
	AEGfxVertexList* pSelectedMesh;
	AEGfxVertexList* pItemMesh;
	s8 fontId;

public:
	Inventory();
	~Inventory();

	void Load();
	void Init();
	void Unload();
	void Update();
	void Draw();

	void AddItem(ItemType type);
	bool UseItem(int slotIndex);
	bool HasKey() const;
	int GetKeyCount() const;  // Add this

private:
	void CreateMeshes();
};

// Global inventory instance
extern Inventory g_Inventory;

#endif // INVENTORY_H