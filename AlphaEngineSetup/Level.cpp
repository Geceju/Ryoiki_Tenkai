#include "Level.h"
#include "SettingsMenu.h"
#include "RoomGenerator.h"
#include "jogo.h" 
#include "GameStateManager.h"
#include "Enemy.h"
#include "Items.h"
#include "Tilesets.h" 
#include "Inventory.h"
#include <cstdio> 
#include <cmath>  
#include <queue> 
#include <map>   
#include <algorithm> 

// items manager static variables
static ItemsManager g_ItemsManager;
static bool g_ItemsInitialized = false;
static int g_PlayerScore = 0;
static bool g_AllItemsCollectedMessageShown = false;

// global variables for the level data
static std::vector<std::unique_ptr<Room>> g_DungeonRooms;

// geometry pointers
static AEGfxVertexList* g_pUnitSquare = nullptr;
static AEGfxVertexList* g_pRectOutline = nullptr;

// entity data
static std::unique_ptr<Character> g_Character = nullptr;

// END ENTITY DATA
//enemies
static std::vector<SimpleEnemy> g_Enemies;
static int g_Difficulty = 1;

// level state
static bool g_RevealNeighbors = true;
static Room* g_BossRoom = nullptr;
static bool g_ShowWayfinder = false;
static bool g_ShowColors = true;
static s8 g_FontId = -1;

// Settings/Pause state
static bool s_ShowSettings = false;

// draw line segment between two points
static void DrawLineSegment(float x1, float y1, float x2, float y2, float thickness, float r, float g, float b, float a)
{
	float dx = x2 - x1;
	float dy = y2 - y1;
	float dist = sqrtf(dx * dx + dy * dy);
	float angle = atan2f(dy, dx);
	float midX = (x1 + x2) / 2.0f;
	float midY = (y1 + y2) / 2.0f;

	AEMtx33 scale, rot, trans, transform;
	AEMtx33Scale(&scale, dist, thickness);
	AEMtx33Rot(&rot, angle);
	AEMtx33Trans(&trans, midX, midY);

	AEMtx33Concat(&transform, &rot, &scale);
	AEMtx33Concat(&transform, &trans, &transform);

	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToMultiply(r, g, b, a);
	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(g_pUnitSquare, AE_GFX_MDM_TRIANGLES);
}

// find sequence of rooms using breadth first search
static std::vector<AEVec2> GetPathToBoss(Room* startRoom, Room* targetRoom)
{
	std::vector<AEVec2> pathPoints;
	if (!startRoom || !targetRoom) return pathPoints;

	std::queue<Room*> frontier;
	frontier.push(startRoom);
	std::map<Room*, Room*> cameFrom;
	cameFrom[startRoom] = nullptr;
	Room* current = nullptr;

	while (!frontier.empty())
	{
		current = frontier.front();
		frontier.pop();
		if (current == targetRoom) break;

		int midX = current->tileCountX / 2;
		int midY = current->tileCountY / 2;

		for (Room* next : current->GetNeighbours())
		{
			if (cameFrom.find(next) == cameFrom.end())
			{
				bool hasDoor = false;
				AEVec2 currentCenter = current->rect.GetCenter();
				AEVec2 nextCenter = next->rect.GetCenter();

				if (nextCenter.y > currentCenter.y) { if (current->tileMap[0][midX] == 0) hasDoor = true; }
				else if (nextCenter.y < currentCenter.y) { if (current->tileMap[current->tileCountY - 1][midX] == 0) hasDoor = true; }
				else if (nextCenter.x > currentCenter.x) { if (current->tileMap[midY][current->tileCountX - 1] == 0) hasDoor = true; }
				else if (nextCenter.x < currentCenter.x) { if (current->tileMap[midY][0] == 0) hasDoor = true; }

				if (hasDoor)
				{
					frontier.push(next);
					cameFrom[next] = current;
				}
			}
		}
	}

	if (current == targetRoom)
	{
		Room* step = targetRoom;
		while (step != nullptr)
		{
			pathPoints.push_back(step->rect.GetCenter());
			step = cameFrom[step];
		}
		std::reverse(pathPoints.begin(), pathPoints.end());
	}
	return pathPoints;
}

void Level_Load()
{
	AEGfxMeshStart();
	AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	g_pUnitSquare = AEGfxMeshEnd();

	AEGfxMeshStart();
	AEGfxVertexAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
	AEGfxVertexAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f);
	AEGfxVertexAdd(0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f);
	AEGfxVertexAdd(-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxVertexAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
	g_pRectOutline = AEGfxMeshEnd();

	g_FontId = AEGfxCreateFont("Assets/exo2-regular.ttf", 20);
	if (g_FontId < 0) g_FontId = AEGfxCreateFont("Assets\\exo2-regular.ttf", 20);

	// Initialize items graphics (AFTER engine is ready)
	g_ItemsManager.InitializeGraphics();
	g_Inventory.Load();  // ADD THIS

	// Load shared settings menu resources
	SettingsMenu_Load();
}

void Level_Init()
{
	RoomGenerator generator;
	g_DungeonRooms = generator.Generate(3072, 3072, 512);

	Room* startRoom = nullptr;
	g_BossRoom = nullptr;

	for (const auto& room : g_DungeonRooms)
	{
		if (room->type == RoomType::Start) startRoom = room.get();
		else if (room->type == RoomType::Boss) g_BossRoom = room.get();
	}

	if (startRoom)
	{
		float tileSize = startRoom->rect.width() / 16.0f;
		int startGridX = static_cast<int>(startRoom->rect.GetCenter().x / tileSize);
		int startGridY = static_cast<int>(startRoom->rect.GetCenter().y / tileSize);

		g_Character = std::make_unique<Character>(startGridX, startGridY, tileSize);
		g_Character->Load();
		startRoom->isDiscovered = true;

		if (g_RevealNeighbors)
		{
			for (auto* neighbor : startRoom->GetNeighbours())
			{
				if (neighbor) neighbor->isDiscovered = true;
			}
		}
	}
	// --- SPAWN ENEMIES ---
	g_Enemies.clear();

	int totalEnemiesToSpawn = 3;
	int spawnedCount = 0;

	while (spawnedCount < totalEnemiesToSpawn)
	{
		int roomIdx = Random::Range(0, (int)g_DungeonRooms.size() - 1);
		auto& room = g_DungeonRooms[roomIdx];

		// Safety: Don't spawn in the Start room
		if (room->type == RoomType::Start) continue;

		// change to 1, 512 for new map
		int randCol = Random::Range(1, 14);
		int randRow = Random::Range(1, 14);

		if (room->tileMap[randRow][randCol] == 0) // It's a floor!
		{
			float spawnX = room->rect.left + (randCol * room->tileSize) + (room->tileSize * 0.5f);
			float spawnY = room->rect.top - (randRow * room->tileSize) - (room->tileSize * 0.5f);

			SimpleEnemy newEnemy;
			newEnemy.Load();
			newEnemy.SetPosition(spawnX, spawnY);

			// Dice roll for chase duration (e.g., 1d6 roll + base time)
			int diceRoll = Random::Range(1, 6);

			newEnemy.SetChaseDuration(10.0f + static_cast<float>(diceRoll) * 5.0f);

			newEnemy.currentState = EnemyState::PATROL; // Start patrolling

			g_Enemies.push_back(newEnemy);
			spawnedCount++;
		}
	}


	if (!g_ItemsInitialized)
	{
		// Spawn regular items (3 types)
		for (const auto& room : g_DungeonRooms)
		{
			if (room->type == RoomType::Start || room->type == RoomType::Boss) continue;
			if (std::rand() % 100 < 30)
			{
				int randomType = std::rand() % 3; // Only 0-2 for regular items
				g_ItemsManager.SpawnItem(room->rect.GetCenter().x, room->rect.GetCenter().y, (ItemType)randomType);
			}
		}

		// --- SPAWN A KEY in a random non-start, non-boss room ---
		std::vector<Room*> eligibleRooms;
		for (const auto& room : g_DungeonRooms)
		{
			if (room->type != RoomType::Start && room->type != RoomType::Boss)
			{
				eligibleRooms.push_back(room.get());
			}
		}

		if (!eligibleRooms.empty())
		{
			int randomRoomIndex = std::rand() % eligibleRooms.size();
			Room* keyRoom = eligibleRooms[randomRoomIndex];

			// Find a valid floor tile in that room
			int attempts = 0;
			bool keyPlaced = false;

			while (!keyPlaced && attempts < 100)
			{
				int tileX = std::rand() % 16; // Assuming 16x16 rooms
				int tileY = std::rand() % 16;

				if (keyRoom->tileMap[tileY][tileX] == 0) // Floor tile
				{
					float keyX = keyRoom->rect.left + (tileX * keyRoom->tileSize) + (keyRoom->tileSize * 0.5f);
					float keyY = keyRoom->rect.top - (tileY * keyRoom->tileSize) - (keyRoom->tileSize * 0.5f);

					g_ItemsManager.SpawnItem(keyX, keyY, ItemType::KEY);
					keyPlaced = true;
					printf("Key spawned in room at (%f, %f)\n", keyX, keyY);
				}
				attempts++;
			}

			if (!keyPlaced)
			{
				// Fallback: place key at room center
				g_ItemsManager.SpawnItem(keyRoom->rect.GetCenter().x, keyRoom->rect.GetCenter().y, ItemType::KEY);
				printf("Key spawned at room center\n");
			}
		}

		g_ItemsInitialized = true;
	}

	g_Inventory.Init();

	s_ShowSettings = false;
	SettingsMenu_Initialize();
}

void Level_Update()
{
	// Toggle Settings/Pause
	if (AEInputCheckTriggered(AEVK_ESCAPE))
	{
		s_ShowSettings = !s_ShowSettings;
	}

	if (s_ShowSettings)
	{
		SettingsMenu_Update(s_ShowSettings);
		return; // Stop game logic while menu is open
	}

	// Normal Level Updates
	if (AEInputCheckTriggered(AEVK_R)) gGameStateNext = GS_RESTART;
	if (AEInputCheckTriggered(AEVK_Q)) gGameStateNext = GS_QUIT;
	if (AEInputCheckTriggered(AEVK_N)) g_RevealNeighbors = !g_RevealNeighbors;
	if (AEInputCheckTriggered(AEVK_M)) g_ShowWayfinder = !g_ShowWayfinder;
	if (AEInputCheckTriggered(AEVK_C)) g_ShowColors = !g_ShowColors;

	// Update inventory (handles number key presses)
	g_Inventory.Update();  // ADD THIS

	if (g_Character)
	{
		g_Character->Update(g_DungeonRooms);
		g_Character->CheckItemCollection(g_ItemsManager);
		AEGfxSetCamPosition(g_Character->GetWorldX(), g_Character->GetWorldY());

		// Room Discovery
		for (auto& room : g_DungeonRooms)
		{
			if (g_Character->GetWorldX() >= room->rect.left && g_Character->GetWorldX() <= room->rect.right &&
				g_Character->GetWorldY() >= room->rect.bottom && g_Character->GetWorldY() <= room->rect.top)
			{
				room->isDiscovered = true;
				if (g_RevealNeighbors)
				{
					for (auto* neighbor : room->GetNeighbours()) if (neighbor) neighbor->isDiscovered = true;
				}
				break;
			}
		}
	}

	float dt = (float)AEFrameRateControllerGetFrameTime();

	// Call the function from the new file
	for (auto& enemy : g_Enemies)
	{
		// 1. Update enemy AI/Movement first
		enemy.Update(g_Character->GetWorldX(), g_Character->GetWorldY(), dt, g_DungeonRooms);

	};

	// Debug: Show item count when 'I' is pressed
	if (AEInputCheckTriggered(AEVK_I))
	{
		printf("Items collected: %d/%d\n",
			g_ItemsManager.GetCollectedCount(),
			g_ItemsManager.GetTotalCount());

		// Debug: Print positions of all uncollected items
		printf("Uncollected items at positions:\n");
		// You might need to add a method to ItemsManager to iterate items
	}
}

void Level_Draw()
{
	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);

	// Draw Rooms
	for (const auto& room : g_DungeonRooms)
	{
		if (!room->isDiscovered) continue;
		const TilesetData& style = TilesetManager::Get(room->tilesetID);

		AEMtx33 scale, trans, transform;
		AEMtx33Scale(&scale, (float)room->rect.width(), (float)room->rect.height());
		AEMtx33Trans(&trans, room->rect.GetCenter().x, room->rect.GetCenter().y);
		AEMtx33Concat(&transform, &trans, &scale);
		AEGfxSetTransform(transform.m);

		if (room->type == RoomType::Boss) AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f);
		else if (!g_ShowColors) AEGfxSetColorToMultiply(0.5f, 0.5f, 0.5f, 1.0f);
		else
		{
			if (room->type == RoomType::Start) AEGfxSetColorToMultiply(0.0f, 1.0f, 0.0f, 1.0f);
			else AEGfxSetColorToMultiply(style.r, style.g, style.b, 1.0f);
		}
		AEGfxMeshDraw(g_pUnitSquare, AE_GFX_MDM_TRIANGLES);

		// Draw Walls
		AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f);
		for (int y = 0; y < room->tileCountY; ++y)
		{
			for (int x = 0; x < room->tileCountX; ++x)
			{
				if (room->tileMap[y][x] == 1)
				{
					float wx = room->rect.left + (x * room->tileSize) + (room->tileSize * 0.5f);
					float wy = room->rect.top - (y * room->tileSize) - (room->tileSize * 0.5f);
					AEMtx33Scale(&scale, room->tileSize, room->tileSize);
					AEMtx33Trans(&trans, wx, wy);
					AEMtx33Concat(&transform, &trans, &scale);
					AEGfxSetTransform(transform.m);
					AEGfxMeshDraw(g_pUnitSquare, AE_GFX_MDM_TRIANGLES);
				}
			}
		}

		if (room->type == RoomType::Boss)
		{
			float camX = g_Character ? g_Character->GetWorldX() : 0.0f;
			float camY = g_Character ? g_Character->GetWorldY() : 0.0f;
			float textX = (room->rect.GetCenter().x - camX) * 2.0f / AEGfxGetWindowWidth();
			float textY = (room->rect.GetCenter().y - camY) * 2.0f / AEGfxGetWindowHeight();
			if (g_FontId >= 0 && textX > -0.9f && textX < 0.9f && textY > -0.9f && textY < 0.9f)
				AEGfxPrint(g_FontId, (char*)"exit", textX, textY, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	// Draw Items
	AEGfxVertexList* itemMesh = g_ItemsManager.GetItemMesh();
	if (itemMesh)
	{
		const auto& allItems = g_ItemsManager.GetItems();
		for (const auto& item : allItems)
		{
			if (item.collected || !item.active) continue;
			AEMtx33 scale, trans, transform;
			AEMtx33Scale(&scale, item.visualRadius * 100.0f, item.visualRadius * 100.0f);
			AEMtx33Trans(&trans, item.x, item.y);
			AEMtx33Concat(&transform, &trans, &scale);
			AEGfxSetColorToMultiply(item.color[0], item.color[1], item.color[2], item.color[3]);
			AEGfxSetTransform(transform.m);
			AEGfxMeshDraw(itemMesh, AE_GFX_MDM_TRIANGLES);
		}
	}
	if (g_Character) g_Character->Draw();
	for (auto& enemy : g_Enemies)
	{
		enemy.Draw();
	}


	// Wayfinder
	if (g_ShowWayfinder && g_BossRoom && g_Character)
	{
		float px = g_Character->GetWorldX(), py = g_Character->GetWorldY();
		Room* playerRoom = nullptr;
		for (const auto& room : g_DungeonRooms)
			if (px >= room->rect.left && px <= room->rect.right && py >= room->rect.bottom && py <= room->rect.top) { playerRoom = room.get(); break; }

		if (playerRoom)
		{
			std::vector<AEVec2> path = GetPathToBoss(playerRoom, g_BossRoom);
			if (!path.empty())
			{
				DrawLineSegment(px, py, path[0].x, path[0].y, 5.0f, 1.0f, 1.0f, 0.0f, 0.5f);
				for (size_t i = 0; i < path.size() - 1; ++i)
					DrawLineSegment(path[i].x, path[i].y, path[i + 1].x, path[i + 1].y, 5.0f, 1.0f, 1.0f, 0.0f, 0.5f);
			}
		}
	}

	// Settings Overlay
	if (s_ShowSettings)
	{
		SettingsMenu_Draw(true); // Draw with "Exit to Menu" button
		// Restore camera for next frame
		if (g_Character) AEGfxSetCamPosition(g_Character->GetWorldX(), g_Character->GetWorldY());
	}

	// Draw inventory last so it appears on top
	g_Inventory.Draw();
}

void Level_Free()
{
	for (auto& room : g_DungeonRooms) if (room) room->ClearNeighbours();
	g_DungeonRooms.clear();
	g_Character = nullptr;
	g_BossRoom = nullptr;
	g_ItemsInitialized = false;
}

void Level_Unload()
{
	if (g_pUnitSquare) { AEGfxMeshFree(g_pUnitSquare); g_pUnitSquare = nullptr; }
	if (g_pRectOutline) { AEGfxMeshFree(g_pRectOutline); g_pRectOutline = nullptr; }

	if (g_FontId >= 0) {
		AEGfxDestroyFont(g_FontId);
		g_FontId = -1;
	}

	// Release any textures or allocated data inside the player class

	for (auto& enemy : g_Enemies) {
		enemy.Unload();
	}
	g_Enemies.clear();

	// 2. Clear the mesh ONCE at the very end
	if (g_pUnitSquare) {
		AEGfxMeshFree(g_pUnitSquare);
		g_pUnitSquare = nullptr;
	}

	g_Inventory.Unload();
	SettingsMenu_Unload();
}