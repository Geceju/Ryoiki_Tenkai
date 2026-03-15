#include "Level.h"
#include "SettingsMenu.h" // Added for global settings access
#include "RoomGenerator.h"
#include "jogo.h" 
#include "GameStateManager.h"
#include "Enemy.h"
#include "Items.h"
#include "Tilesets.h" 
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
static SimpleEnemy g_Enemy;

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

	g_Enemy.Load();
	g_ItemsManager.InitializeGraphics();

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

	if (!g_ItemsInitialized)
	{
		for (const auto& room : g_DungeonRooms)
		{
			if (room->type == RoomType::Start || room->type == RoomType::Boss) continue;
			if (std::rand() % 100 < 30)
			{
				int randomType = std::rand() % 3;
				g_ItemsManager.SpawnItem(room->rect.GetCenter().x, room->rect.GetCenter().y, (ItemType)randomType);
			}
		}
		g_ItemsInitialized = true;
	}

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

	float dt = (float)AEFrameRateControllerGetFrameTime();

	if (g_Character)
	{
		g_Character->Update(g_DungeonRooms);
		g_Character->UpdateAbilities(dt, g_Enemy, g_ItemsManager, g_DungeonRooms);
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

	
	g_Enemy.Update(g_Character->GetWorldX(), g_Character->GetWorldY(), dt, g_DungeonRooms);
	g_ItemsManager.Update(g_Character->GetWorldX(), g_Character->GetWorldY(), 0.0f);
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

	if (g_Character)
	{
		g_Character->DrawAbilities();
		g_Character->Draw();
	}
	g_Enemy.Draw();

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
	if (g_pUnitSquare) AEGfxMeshFree(g_pUnitSquare);
	if (g_pRectOutline) AEGfxMeshFree(g_pRectOutline);
	if (g_FontId >= 0) AEGfxDestroyFont(g_FontId);
	g_Enemy.Unload();
	SettingsMenu_Unload();
}