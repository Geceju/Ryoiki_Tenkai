#include "Inventory.h"
#include <cstdio>

// Define the global inventory instance
Inventory g_Inventory;

Inventory::Inventory()
	: selectedSlot(0)
	, pSlotMesh(nullptr)
	, pSelectedMesh(nullptr)
	, pItemMesh(nullptr)
	, fontId(-1) {

	// Call Init to set up the fixed slots
	Init();
}

Inventory::~Inventory() {
	Unload();
}

void Inventory::Load() {
	// Create font for numbers
	fontId = AEGfxCreateFont("Assets/exo2-regular.ttf", 20);
	if (fontId < 0) {
		fontId = AEGfxCreateFont("Assets\\exo2-regular.ttf", 20);
	}

	CreateMeshes();
}

void Inventory::Init() {
	selectedSlot = 0;

	// FIXING SLOTS: Hardcode each slot to a specific item type for abilities

	// Slot 1 (Index 0): Key 1 (Speed Boost) -> POWER_UP (Blue)
	slots[0].itemType = ItemType::POWER_UP;
	slots[0].count = 0;
	slots[0].isActive = false;

	// Slot 2 (Index 1): Key 2 (Stun) -> SLOW_ENEMY (Purple)
	slots[1].itemType = ItemType::SLOW_ENEMY;
	slots[1].count = 0;
	slots[1].isActive = false;

	// Slot 3 (Index 2): Key 3 (Guide) -> POINT (Red)
	slots[2].itemType = ItemType::POINT;
	slots[2].count = 0;
	slots[2].isActive = false;

	// Slot 4 (Index 3): Key 4 (Exit) -> KEY (Yellow)
	slots[3].itemType = ItemType::KEY;
	slots[3].count = 0;
	slots[3].isActive = false;
}

void Inventory::Unload() {
	if (pSlotMesh) {
		AEGfxMeshFree(pSlotMesh);
		pSlotMesh = nullptr;
	}
	if (pSelectedMesh) {
		AEGfxMeshFree(pSelectedMesh);
		pSelectedMesh = nullptr;
	}
	if (pItemMesh) {
		AEGfxMeshFree(pItemMesh);
		pItemMesh = nullptr;
	}
	if (fontId >= 0) {
		AEGfxDestroyFont(fontId);
		fontId = -1;
	}
}

void Inventory::CreateMeshes() {
	// Create slot mesh (gray square)
	AEGfxMeshStart();
	AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	pSlotMesh = AEGfxMeshEnd();

	// Create selected slot mesh (white outline)
	AEGfxMeshStart();
	AEGfxVertexAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
	AEGfxVertexAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f);
	AEGfxVertexAdd(0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f);
	AEGfxVertexAdd(-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxVertexAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
	pSelectedMesh = AEGfxMeshEnd();

	// Create item mesh (small square for items in slots)
	AEGfxMeshStart();
	AEGfxTriAdd(-0.3f, -0.3f, 0xFFFFFFFF, 0.0f, 1.0f,
		0.3f, -0.3f, 0xFFFFFFFF, 1.0f, 1.0f,
		-0.3f, 0.3f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxTriAdd(0.3f, -0.3f, 0xFFFFFFFF, 1.0f, 1.0f,
		0.3f, 0.3f, 0xFFFFFFFF, 1.0f, 0.0f,
		-0.3f, 0.3f, 0xFFFFFFFF, 0.0f, 0.0f);
	pItemMesh = AEGfxMeshEnd();
}

void Inventory::Update() {
	// Keep the key check for exiting the level, as that isn't an ability
	if (AEInputCheckTriggered(AEVK_4)) {
		UseItem(3);  // Slot 4 for key
	}
}

void Inventory::Draw() {
	if (!pSlotMesh) return;

	// Get camera position
	float camX, camY;
	AEGfxGetCamPosition(&camX, &camY);

	// Get window dimensions
	int windowWidth = AEGfxGetWindowWidth();
	int windowHeight = AEGfxGetWindowHeight();

	// Calculate starting X position (bottom-left with offset)
	float startX = camX - (windowWidth * 0.5f) + X_OFFSET;
	float startY = camY - (windowHeight * 0.5f) + Y_OFFSET;

	// Draw all slots first
	for (int i = 0; i < HOTBAR_SIZE; ++i) {
		float slotX = startX + (float)i * (SLOT_SIZE + SLOT_SPACING);
		float slotY = startY;

		// Center the slot for rendering
		float worldX = slotX + (SLOT_SIZE * 0.5f);
		float worldY = slotY + (SLOT_SIZE * 0.5f);

		AEMtx33 scale, trans, transform;
		AEMtx33Scale(&scale, SLOT_SIZE, SLOT_SIZE);
		AEMtx33Trans(&trans, worldX, worldY);
		AEMtx33Concat(&transform, &trans, &scale);

		// Draw slot background
		if (i == selectedSlot) {
			AEGfxSetColorToMultiply(0.5f, 0.5f, 0.5f, 0.9f); // Lighter gray for selected
		}
		else {
			AEGfxSetColorToMultiply(0.3f, 0.3f, 0.3f, 0.9f); // Darker gray for unselected
		}
		AEGfxSetTransform(transform.m);
		AEGfxMeshDraw(pSlotMesh, AE_GFX_MDM_TRIANGLES);

		// Draw white outline for selected slot
		if (i == selectedSlot) {
			AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
			AEGfxSetTransform(transform.m);
			AEGfxMeshDraw(pSelectedMesh, AE_GFX_MDM_LINES_STRIP);
		}

		// --- DRAW ITEM (Solid if we have it, Faded if we don't) ---

		// Setup size and position for the item
		AEMtx33Scale(&scale, SLOT_SIZE * 0.6f, SLOT_SIZE * 0.6f);
		AEMtx33Trans(&trans, worldX, worldY);
		AEMtx33Concat(&transform, &trans, &scale);

		// Check if the player currently has the item in this slot
		bool hasItem = (slots[i].isActive && slots[i].count > 0);

		// Full opacity if owned, 30% opacity if empty
		float alpha = hasItem ? 1.0f : 0.3f;
		float r = 1.0f, g = 1.0f, b = 1.0f;

		// Set color based on the fixed item type for this slot
		switch (slots[i].itemType) {
		case ItemType::POWER_UP:   r = 0.0f; g = 0.0f; b = 1.0f; break; // Blue
		case ItemType::SLOW_ENEMY: r = 1.0f; g = 0.0f; b = 1.0f; break; // Purple
		case ItemType::POINT:      r = 1.0f; g = 0.0f; b = 0.0f; break; // Red
		case ItemType::KEY:        r = 1.0f; g = 1.0f; b = 0.0f; break; // Yellow
		}

		// If empty, darken the RGB values to make it look grayed-out
		if (!hasItem) {
			r *= 0.5f;
			g *= 0.5f;
			b *= 0.5f;
		}

		// Draw it! (Alpha blending MUST be enabled to see the 30% transparency)
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetColorToMultiply(r, g, b, alpha);
		AEGfxSetTransform(transform.m);
		AEGfxMeshDraw(pItemMesh, AE_GFX_MDM_TRIANGLES);
		AEGfxSetBlendMode(AE_GFX_BM_NONE); // Turn blend mode back off just in case
	}

	// Draw numbers and counts on top of slots
	for (int i = 0; i < HOTBAR_SIZE; ++i) {
		if (fontId >= 0) {
			float slotX = startX + (float)i * (SLOT_SIZE + SLOT_SPACING);
			float slotY = startY;

			// Draw slot number or "KEY" at the TOP of the box
			char slotText[8];

			if (i == 3) {  // Slot 4 (index 3) - show "KEY"
				sprintf_s(slotText, "KEY");
			}
			else {
				sprintf_s(slotText, "%d", i + 1);  // Show 1, 2, 3
			}

			float textX = (slotX + SLOT_SIZE * 0.5f - camX) / (windowWidth * 0.5f);
			float textY = (slotY + SLOT_SIZE * 0.8f - camY) / (windowHeight * 0.5f);

			// ALL TEXT NOW SMALLER AND SAME SIZE (0.7f)
			if (i == 3) {
				AEGfxPrint(fontId, slotText, textX, textY, 0.7f, 1.0f, 1.0f, 0.0f, 1.0f); // Yellow for KEY
			}
			else {
				AEGfxPrint(fontId, slotText, textX, textY, 0.7f, 1.0f, 1.0f, 1.0f, 1.0f); // White for numbers (now 0.7f)
			}

			// Draw item count at the BOTTOM RIGHT of the box if more than 1
			if (slots[i].isActive && slots[i].count > 1) {
				char countText[8];
				sprintf_s(countText, "%d", slots[i].count);

				float countX = (slotX + SLOT_SIZE * 0.8f - camX) / (windowWidth * 0.5f);
				float countY = (slotY + SLOT_SIZE * 0.2f - camY) / (windowHeight * 0.5f);

				// Count numbers also smaller (0.7f)
				AEGfxPrint(fontId, countText, countX, countY, 0.7f, 1.0f, 1.0f, 1.0f, 1.0f);
			}
		}
	}
}

void Inventory::AddItem(ItemType type) {
	int targetSlot = -1;

	// Map each item strictly to its dedicated fixed slot
	switch (type) {
	case ItemType::POWER_UP:
		targetSlot = 0; // Slot 1
		break;
	case ItemType::SLOW_ENEMY:
		targetSlot = 1; // Slot 2
		break;
	case ItemType::POINT:
		targetSlot = 2; // Slot 3
		break;
	case ItemType::KEY:
		targetSlot = 3; // Slot 4
		break;
	}

	// If a valid slot was found, add it
	if (targetSlot != -1) {
		slots[targetSlot].isActive = true;
		slots[targetSlot].count++;

		if (type == ItemType::KEY) {
			printf("KEY added to slot 4! (%d/3)\n", slots[targetSlot].count);
		}
		else {
			printf("Item added to fixed slot %d! Count: %d\n", targetSlot + 1, slots[targetSlot].count);
		}
	}
}

bool Inventory::UseItem(int slotIndex) {
	if (slotIndex < 0 || slotIndex >= HOTBAR_SIZE) return false;

	InventorySlot& slot = slots[slotIndex];

	if (!slot.isActive || slot.count <= 0) return false;

	// Don't allow using the key - it's just for exiting
	if (slot.itemType == ItemType::KEY) {
		printf("You have %d/3 keys. Press E at the exit to use them.\n", slot.count);
		return false;
	}

	slot.count--;
	selectedSlot = slotIndex;

	// Apply effect
	switch (slot.itemType) {
	case ItemType::POINT:
		printf("Used Point! (+10 points) - %d remaining\n", slot.count);
		break;
	case ItemType::POWER_UP:
		printf("Used Power-Up! (speed boost) - %d remaining\n", slot.count);
		break;
	case ItemType::SLOW_ENEMY:
		printf("Used Slow! (enemies slowed) - %d remaining\n", slot.count);
		break;
	}

	if (slot.count <= 0) {
		slot.isActive = false;
		printf("Slot %d is now empty\n", slotIndex + 1);
	}

	return true;
}

bool Inventory::HasKey() const {
	return slots[3].isActive && slots[3].itemType == ItemType::KEY && slots[3].count > 0;
}

int Inventory::GetKeyCount() const {
	if (slots[3].isActive && slots[3].itemType == ItemType::KEY) {
		return slots[3].count;
	}
	return 0;
}

// --- NEW METHODS ---

// Check if an item exists in the inventory without using it
bool Inventory::HasItem(ItemType type) const {
	for (int i = 0; i < HOTBAR_SIZE; ++i) {
		if (slots[i].isActive && slots[i].itemType == type && slots[i].count > 0) {
			return true;
		}
	}
	return false;
}

// Consume an item from the inventory (returns true if successful)
bool Inventory::ConsumeItem(ItemType type) {
	for (int i = 0; i < HOTBAR_SIZE; ++i) {
		if (slots[i].isActive && slots[i].itemType == type && slots[i].count > 0) {
			slots[i].count--;
			selectedSlot = i; // Optionally highlight the slot being used

			printf("Used item of type %d! - %d remaining\n", (int)type, slots[i].count);

			if (slots[i].count <= 0) {
				slots[i].isActive = false;
				printf("Slot %d is now empty\n", i + 1);
			}
			return true; // Successfully consumed
		}
	}
	return false; // Item not found or count is 0
}