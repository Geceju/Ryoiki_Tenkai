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
	// Check for number key presses (1, 2, 3)
	if (AEInputCheckTriggered(AEVK_1)) {
		UseItem(0);
	}
	else if (AEInputCheckTriggered(AEVK_2)) {
		UseItem(1);
	}
	else if (AEInputCheckTriggered(AEVK_3)) {
		UseItem(2);
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
	// We add camera position to make it follow the camera
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

			// Draw slot number at the TOP of the box
			char numText[4];
			sprintf_s(numText, "%d", i + 1);

			// Convert world position to screen space for text
			// Screen space: -1 to 1, with origin at center
			float textX = (slotX + SLOT_SIZE * 0.5f - camX) / (windowWidth * 0.5f);
			float textY = (slotY + SLOT_SIZE * 0.8f - camY) / (windowHeight * 0.5f);

			AEGfxPrint(fontId, numText, textX, textY, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

			// Draw item count at the BOTTOM RIGHT of the box if more than 1
			if (slots[i].isActive && slots[i].count > 1) {
				char countText[8];
				sprintf_s(countText, "%d", slots[i].count);

				// Position at bottom right
				float countX = (slotX + SLOT_SIZE * 0.8f - camX) / (windowWidth * 0.5f);
				float countY = (slotY + SLOT_SIZE * 0.2f - camY) / (windowHeight * 0.5f);

				AEGfxPrint(fontId, countText, countX, countY, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
			}
		}
	}
}

void Inventory::AddItem(ItemType type) {
	// Try to stack with existing item of same type
	for (int i = 0; i < HOTBAR_SIZE; ++i) {
		if (slots[i].isActive && slots[i].itemType == type) {
			slots[i].count++;
			printf("Added to slot %d, now have %d\n", i + 1, slots[i].count);
			return;
		}
	}

	// Find empty slot
	for (int i = 0; i < HOTBAR_SIZE; ++i) {
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