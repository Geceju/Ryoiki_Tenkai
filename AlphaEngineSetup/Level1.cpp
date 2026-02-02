#include "GameStateManager.h"
#include <iostream>
#include "Items.h"
#include "jogo.h"

AEGfxVertexList* pWallMesh = nullptr; // The mesh shape for buttons
Character* player = nullptr;
float TileSize = 48;

std::array<std::array<int, 20>, 15> maze = { {
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1},
	{1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1},
	{1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1},
	{1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1},
	{1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},
	{1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1},
	{1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1},
	{1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1},
	{1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1},
	{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
} };

// Convert maze to vector<vector<int>> for ItemsManager
std::vector<std::vector<int>> ConvertMazeToVector() {
	std::vector<std::vector<int>> result;
	for (const auto& row : maze) {
		result.emplace_back(row.begin(), row.end());
	}
	return result;
}

// Global items manager
ItemsManager* itemsManager = nullptr;

void Level1_Load() {

	AEGfxMeshStart();
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
	pWallMesh = AEGfxMeshEnd();

	std::cout << "Level1 Loaded - Items Test\n";

    player = new Character(1, 1, TileSize);
    player->Load();
}
void Level1_Initialize() {
	std::cout << "Initializing Items Manager (3 items only)...\n";

	// Create items manager
	if (itemsManager == nullptr) {
		itemsManager = new ItemsManager();
	}

	// Initialize with maze dimensions
	auto mazeVector = ConvertMazeToVector();
	itemsManager->Initialize(20, 15, mazeVector, TileSize);

	// Spawn 15 random items for testing (only 3 types)
	itemsManager->SpawnRandomItems(15, mazeVector);

	std::cout << "Spawned " << itemsManager->GetTotalCount() << " random items in the maze\n";

	// DEBUG: Spawn one of each type in a row at the top for easy viewing
	itemsManager->SpawnItem(1.0f, 1.0f, ItemType::POINT);        // RED
	itemsManager->SpawnItem(3.0f, 1.0f, ItemType::POWER_UP);     // BLUE
	itemsManager->SpawnItem(5.0f, 1.0f, ItemType::SLOW_ENEMY);   // PURPLE

	std::cout << "\nDEBUG: Spawned test items at:\n";
	std::cout << "(1,1): RED (POINT)\n";
	std::cout << "(3,1): BLUE (POWER_UP)\n";
	std::cout << "(5,1): PURPLE (SLOW_ENEMY)\n\n";

	// Set background to dark gray for better visibility
	AEGfxSetBackgroundColor(0.1f, 0.1f, 0.1f);
}
void Level1_Update() {
	// Simple test: Press R to respawn items
	if (AEInputCheckTriggered(AEVK_R)) {
		std::cout << "Respawning items...\n";
		itemsManager->Reset();
		auto mazeVector = ConvertMazeToVector();
		itemsManager->SpawnRandomItems(20, mazeVector);
	}
    if (player) {
        player->Update(maze);

		if (itemsManager) {
			player->CollectItem(*itemsManager);
		}
    }

	// Press ESC to quit
	if (AEInputCheckTriggered(AEVK_ESCAPE)) {
		gGameStateNext = GS_MAINMENU;
	}
}

void DrawWall(int x, int y) {
	float drawX = (x * TileSize) - (f32)AEGfxGetWindowWidth() / 2.0f + (TileSize / 2.0f);
	float drawY = (y * TileSize) - (f32)AEGfxGetWindowHeight() / 2.0f + (TileSize / 2.0f);

	AEMtx33 scale, trans, transform;
	AEMtx33Scale(&scale, TileSize, TileSize);
	AEMtx33Trans(&trans, drawX, drawY);
	AEMtx33Concat(&transform, &trans, &scale);

	// Set wall color to white
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetBlendColor(1, 1, 1, 1);
	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(pWallMesh, AE_GFX_MDM_TRIANGLES);
}
void Level1_Draw() {

	// Clear with dark background
	AEGfxSetBackgroundColor(0.1f, 0.1f, 0.1f);

	for (int y = 0; y < 15; ++y) {
		for (int x = 0; x < 20; ++x) {
			if (maze[y][x] == 1) {
				AEGfxSetBackgroundColor(0.2f, 0.2f, 0.2f);
				// Define temporary matrices for calculation
				AEMtx33 scale, trans, transform;

				// Set Render Mode
				AEGfxSetRenderMode(AE_GFX_RM_COLOR);
				// If textures are turned off, we use BlendColor or ColorToMultiply to set the color
				AEGfxSetBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
				// 2. Calculate World Position
				// Offset by -WINDOW_WIDTH/2 if your 0,0 is at the screen center
				float drawX = (x * TileSize) - (f32)AEGfxGetWindowWidth() / 2.0f + (TileSize / 2.0f);
				float drawY = (y * TileSize) - (f32)AEGfxGetWindowHeight() / 2.0f + (TileSize / 2.0f);

				// 3. Build the Transformation Matrix
				AEMtx33Scale(&scale, TileSize, TileSize);
				AEMtx33Trans(&trans, drawX, drawY);

				// Multiply them: transform = trans * rot * scale
				AEMtx33Concat(&transform, &trans, &scale);

				// 4. Send the matrix to Alpha Engine and Draw
				AEGfxSetTransform(transform.m);
				AEGfxMeshDraw(pWallMesh, AE_GFX_MDM_TRIANGLES);
			}
		}
	}

	//Reset blend color to WHITE before drawing items
	AEGfxSetBlendColor(1.0f, 1.0f, 1.0f, 1.0f);

	// Draw all items
	if (itemsManager) {
		itemsManager->Draw();
	}

	if (player) {
        player->Draw();
    }

}

void Level1_Free() {
	if (itemsManager != nullptr) {
		// Force the internal vector to die
		itemsManager->GetItems().clear();
		itemsManager->GetItems().shrink_to_fit();

		delete itemsManager;
		itemsManager = nullptr;
	}

	if (player) {
		delete player;
		player = nullptr;
	}

	// Cleanup Mesh
	if (pWallMesh) {
		AEGfxMeshFree(pWallMesh);
		pWallMesh = nullptr;
	}
}
//void Level1_Free() {
//    if (player) {
//		player->Unload();
//        delete player;
//        player = nullptr;
//    }
//
//	if (pWallMesh) {
//		AEGfxMeshFree(pWallMesh);
//		pWallMesh = nullptr;
//	}
//
//	if (itemsManager) {
//		itemsManager->GetItems().clear();
//		itemsManager->GetItems().shrink_to_fit();
//
//		delete itemsManager;
//		itemsManager = nullptr;
//	}
//}
void Level1_Unload() {
	std::cout << "Level1 Unloading\n";
}