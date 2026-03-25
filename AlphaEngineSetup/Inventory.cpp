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

	// Initialize all slots
	for (int i = 0; i < HOTBAR_SIZE; ++i) {
		slots[i] = InventorySlot();
	}
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
	for (int i = 0; i < HOTBAR_SIZE; ++i) {
		slots[i] = InventorySlot();
	}
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
	// Check for number key presses (1, 2, 3, 4)
	if (AEInputCheckTriggered(AEVK_1)) {
		UseItem(0);
	}
	else if (AEInputCheckTriggered(AEVK_2)) {
		UseItem(1);
	}
	else if (AEInputCheckTriggered(AEVK_3)) {
		UseItem(2);
	}
	else if (AEInputCheckTriggered(AEVK_4)) {
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

		// Draw item if slot has one
		if (slots[i].isActive && slots[i].count > 0) {
			// Draw item icon (slightly smaller than slot)
			AEMtx33Scale(&scale, SLOT_SIZE * 0.6f, SLOT_SIZE * 0.6f);
			AEMtx33Trans(&trans, worldX, worldY);
			AEMtx33Concat(&transform, &trans, &scale);

			switch (slots[i].itemType) {
			case ItemType::POINT:
				AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f); // Red
				break;
			case ItemType::POWER_UP:
				AEGfxSetColorToMultiply(0.0f, 0.0f, 1.0f, 1.0f); // Blue
				break;
			case ItemType::SLOW_ENEMY:
				AEGfxSetColorToMultiply(1.0f, 0.0f, 1.0f, 1.0f); // Purple
				break;
			case ItemType::KEY:
				AEGfxSetColorToMultiply(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
				break;
			}
			AEGfxSetTransform(transform.m);
			AEGfxMeshDraw(pItemMesh, AE_GFX_MDM_TRIANGLES);
		}
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
	// Special handling for KEY - always go to slot 4 (index 3)
	if (type == ItemType::KEY) {
		// Key always goes to slot 4 and stacks there
		if (!slots[3].isActive) {
			slots[3].isActive = true;
			slots[3].itemType = type;
			slots[3].count = 1;
			printf("KEY added to slot 4! (1/3)\n");
		}
		else {
			// Stack keys in slot 4
			slots[3].count++;
			printf("KEY added to slot 4! (%d/3)\n", slots[3].count);
		}
		return;
	}

	// For other items, try to stack with existing item of same type
	for (int i = 0; i < HOTBAR_SIZE - 1; ++i) { // Exclude slot 4 (key slot)
		if (slots[i].isActive && slots[i].itemType == type) {
			slots[i].count++;
			printf("Added to slot %d, now have %d\n", i + 1, slots[i].count);
			return;
		}
	}

	// Find empty slot (excluding slot 4 which is reserved for key)
	for (int i = 0; i < HOTBAR_SIZE - 1; ++i) {
		if (!slots[i].isActive) {
			slots[i].isActive = true;
			slots[i].itemType = type;
			slots[i].count = 1;
			printf("New item in slot %d\n", i + 1);
			return;
		}
	}

	printf("Inventory full!\n");
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