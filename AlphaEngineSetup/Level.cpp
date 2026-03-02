#include "Level.h"
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
static int g_PlayerScore = 0;  // Track player score from items
static bool g_AllItemsCollectedMessageShown = false;  // Prevent message spam

// Global variables for the level data
static std::vector<std::unique_ptr<Room>> g_DungeonRooms;

// Geometry pointers
static AEGfxVertexList* g_pUnitSquare = nullptr;
static AEGfxVertexList* g_pRectOutline = nullptr;

// END GEOMETRY DATA

// ENTITY DATA
// The player object maintains its own state, including position and discovery logic
// Initializing with a tile size of 256 to match the dungeon generation

static std::unique_ptr<Character> g_Character = nullptr;

// END ENTITY DATA

static SimpleEnemy g_Enemy;
// Set this to true to see neighbors, false to see only the current room
static bool g_RevealNeighbors = true;

// Wayfinder variables
static Room* g_BossRoom = nullptr;
static bool g_ShowWayfinder = false;

// Font ID for text rendering
static s8 g_FontId = -1;

// Helper to draw a line segment between two points
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

// Helper to find the sequence of rooms from start to end using BFS
// Now checks for actual doors/openings in the tilemap
static std::vector<AEVec2> GetPathToBoss(Room* startRoom, Room* targetRoom)
{
	std::vector<AEVec2> pathPoints;

	// Validate inputs
	if (!startRoom || !targetRoom)
	{
		return pathPoints;
	}

	// BFS initialization
	std::queue<Room*> frontier;
	frontier.push(startRoom);

	// Map to track visited nodes and path history
	std::map<Room*, Room*> cameFrom;
	cameFrom[startRoom] = nullptr;

	Room* current = nullptr;

	// Run BFS loop
	while (!frontier.empty())
	{
		current = frontier.front();
		frontier.pop();

		// Check if we reached the target
		if (current == targetRoom)
		{
			break;
		}

		int midX = current->tileCountX / 2;
		int midY = current->tileCountY / 2;
		int w = current->tileCountX;
		int h = current->tileCountY;

		// Explore neighbors
		for (Room* next : current->GetNeighbours())
		{
			// If neighbor has not been visited yet
			if (cameFrom.find(next) == cameFrom.end())
			{
				// GEOMETRIC CHECK
				// Even if rooms are next to each other we must check if a door exists
				bool hasDoor = false;

				AEVec2 currentCenter = current->rect.GetCenter();
				AEVec2 nextCenter = next->rect.GetCenter();

				// Check Top Neighbor
				if (nextCenter.y > currentCenter.y)
				{
					// Check top row middle tile
					if (current->tileMap[0][midX] == 0) hasDoor = true;
				}
				// Check Bottom Neighbor
				else if (nextCenter.y < currentCenter.y)
				{
					// Check bottom row middle tile
					if (current->tileMap[h - 1][midX] == 0) hasDoor = true;
				}
				// Check Right Neighbor
				else if (nextCenter.x > currentCenter.x)
				{
					// Check right column middle tile
					if (current->tileMap[midY][w - 1] == 0) hasDoor = true;
				}
				// Check Left Neighbor
				else if (nextCenter.x < currentCenter.x)
				{
					// Check left column middle tile
					if (current->tileMap[midY][0] == 0) hasDoor = true;
				}

				// Only add to queue if there is a valid path
				if (hasDoor)
				{
					frontier.push(next);
					cameFrom[next] = current;
				}
			}
		}
	}

	// Reconstruct path if target found
	if (current == targetRoom)
	{
		Room* step = targetRoom;
		while (step != nullptr)
		{
			pathPoints.push_back(step->rect.GetCenter());
			step = cameFrom[step];
		}
		// Path is built backwards so reverse it
		std::reverse(pathPoints.begin(), pathPoints.end());
	}

	return pathPoints;
}

// Prepares GPU resources
void Level_Load()
{
	//_CrtSetBreakAlloc(278);

	// Create unit square mesh for floors
	AEGfxMeshStart();
	AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	g_pUnitSquare = AEGfxMeshEnd();

	// Create outline mesh for walls
	AEGfxMeshStart();
	AEGfxVertexAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
	AEGfxVertexAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f);
	AEGfxVertexAdd(0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f);
	AEGfxVertexAdd(-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxVertexAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
	g_pRectOutline = AEGfxMeshEnd();

	// If that fails try just the filename in case the working directory is already inside Assets
	if (g_FontId < 0)
	{
		g_FontId = AEGfxCreateFont("exo2-regular.ttf", 20);
	}

	// Enemy load
	g_Enemy.Load();

	// Initialize items graphics (AFTER engine is ready)
	g_ItemsManager.InitializeGraphics();
}

// Generates the level and spawns the player
void Level_Init()
{
	RoomGenerator generator;
	// Generate the dungeon layout with 4096 size and 256 room size
	g_DungeonRooms = generator.Generate(4096, 4096, 256);

	// Reset pointers
	Room* startRoom = nullptr;
	g_BossRoom = nullptr;

	// Locate Start and Boss rooms
	for (const auto& room : g_DungeonRooms)
	{
		if (room->type == RoomType::Start)
		{
			startRoom = room.get();
		}
		else if (room->type == RoomType::Boss)
		{
			g_BossRoom = room.get();
		}
	}

	if (startRoom)
	{
		// Calculate the tile size used by the rooms 
		float tileSize = startRoom->rect.width() / 16.0f;

		// Convert the float center position to integer grid coordinates
		int startGridX = static_cast<int>(startRoom->rect.GetCenter().x / tileSize);
		int startGridY = static_cast<int>(startRoom->rect.GetCenter().y / tileSize);

		// Initialize the character with the calculated grid coordinates
		g_Character = std::make_unique<Character>(startGridX, startGridY, tileSize);
		g_Character->Load();

		// Reveal the start room immediately
		startRoom->isDiscovered = true;

		// Optionally reveal neighbors
		if (g_RevealNeighbors)
		{
			for (auto* neighbor : startRoom->GetNeighbours())
			{
				if (neighbor)
				{
					neighbor->isDiscovered = true;
				}
			}
		}
	}

	// Spawn Items
	if (!g_ItemsInitialized)
	{
		for (const auto& room : g_DungeonRooms)
		{
			AEVec2 center = room->rect.GetCenter();

			// Skip if too close to start or boss room
			if (room->type == RoomType::Start || room->type == RoomType::Boss)
				continue;

			// Random chance to spawn item (60% chance per room)
			if (std::rand() % 100 < 30)
			{
				// Random item type
				int randomType = std::rand() % 3;
				ItemType type = static_cast<ItemType>(randomType);

				// Spawn at room center
				g_ItemsManager.SpawnItem(center.x, center.y, type);
			}
		}

		for (int i = 0; i < 20; ++i)
		{
			if (!g_DungeonRooms.empty())
			{
				int roomIndex = std::rand() % g_DungeonRooms.size();
				auto& room = g_DungeonRooms[roomIndex];

				if (room->type == RoomType::Normal) // Only normal rooms
				{
					AEVec2 center = room->rect.GetCenter();

					// Random offset within room (avoid edges)
					float offsetX = (std::rand() % 100 - 50) * 2.0f; // �100 pixels
					float offsetY = (std::rand() % 100 - 50) * 2.0f;

					int randomType = std::rand() % 3;
					ItemType type = static_cast<ItemType>(randomType);

					g_ItemsManager.SpawnItem(center.x + offsetX, center.y + offsetY, type);
				}
			}
		}

		g_ItemsInitialized = true;
		printf("Spawned %d items in world coordinates\n", g_ItemsManager.GetTotalCount());
	}
}

// Updates game logic per frame
void Level_Update()
{
	// Restart Level
	if (AEInputCheckTriggered(AEVK_R))
	{
		gGameStateNext = GS_RESTART;
	}

	// Quit Game
	if (AEInputCheckTriggered(AEVK_Q))
	{
		gGameStateNext = GS_QUIT;
	}

	// Return to Main Menu
	if (AEInputCheckTriggered(AEVK_ESCAPE))
	{
		gGameStateNext = GS_MAINMENU;
	}

	// Toggle Neighbor Reveal Debug
	if (AEInputCheckTriggered(AEVK_N))
	{
		g_RevealNeighbors = !g_RevealNeighbors;
		printf("Neighbor Reveal: %s\n", g_RevealNeighbors ? "ON" : "OFF");
	}

	// Toggle Boss Wayfinder
	if (AEInputCheckTriggered(AEVK_M))
	{
		g_ShowWayfinder = !g_ShowWayfinder;
		printf("Wayfinder: %s\n", g_ShowWayfinder ? "ON" : "OFF");
	}

	// Update the character logic passing the room list for collision
	if (g_Character)
	{
		g_Character->Update(g_DungeonRooms);

		g_Character->CheckItemCollection(g_ItemsManager);

		// Update camera position to follow the character
		AEGfxSetCamPosition(g_Character->GetWorldX(), g_Character->GetWorldY());

		// Update neighbor discovery logic based on character world position
		for (auto& room : g_DungeonRooms)
		{
			// Check if character is inside the room bounds
			if (g_Character->GetWorldX() >= room->rect.left && g_Character->GetWorldX() <= room->rect.right &&
				g_Character->GetWorldY() >= room->rect.bottom && g_Character->GetWorldY() <= room->rect.top)
			{
				room->isDiscovered = true;

				if (g_RevealNeighbors)
				{
					for (auto* neighbor : room->GetNeighbours())
					{
						if (neighbor)
						{
							neighbor->isDiscovered = true;
						}
					}
				}
				break;
			}
		}
	}


	// Update items with player's WORLD position
	float playerWorldX = g_Character->GetWorldX();
	float playerWorldY = g_Character->GetWorldY();
	float dt = (float)AEFrameRateControllerGetFrameTime();

	// Call the function from the new file
	g_Enemy.Update(g_Character->GetWorldX(), g_Character->GetWorldY(), dt, g_DungeonRooms);
	g_ItemsManager.Update(playerWorldX, playerWorldY, 0.0f);

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

// Renders the scene
void Level_Draw()
{
	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);

	// Draw Rooms and Contents
	for (const auto& room : g_DungeonRooms)
	{
		// Skip hidden rooms
		if (!room->isDiscovered) continue;

		const TilesetData& style = TilesetManager::Get(room->tilesetID);

		// DRAW FLOOR
		AEMtx33 scale, trans, transform;
		AEMtx33Scale(&scale, (float)room->rect.width(), (float)room->rect.height());
		AEMtx33Trans(&trans, room->rect.GetCenter().x, room->rect.GetCenter().y);
		AEMtx33Concat(&transform, &trans, &scale);
		AEGfxSetTransform(transform.m);

		// Set color based on room type
		if (room->type == RoomType::Boss) AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f);
		else if (room->type == RoomType::Start) AEGfxSetColorToMultiply(0.0f, 1.0f, 0.0f, 1.0f);
		else AEGfxSetColorToMultiply(style.r, style.g, style.b, 1.0f);

		AEGfxMeshDraw(g_pUnitSquare, AE_GFX_MDM_TRIANGLES);

		// DRAW WALLS
		AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f);

		for (int y = 0; y < room->tileCountY; ++y)
		{
			for (int x = 0; x < room->tileCountX; ++x)
			{
				// If the tile is marked as 1 it is a wall
				if (room->tileMap[y][x] == 1)
				{
					// Calculate World Position of this specific tile
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

		// DRAW LABEL FOR BOSS ROOM
		if (room->type == RoomType::Boss)
		{
			float camX = g_Character ? g_Character->GetWorldX() : 0.0f;
			float camY = g_Character ? g_Character->GetWorldY() : 0.0f;
			float winW = (float)AEGfxGetWindowWidth();
			float winH = (float)AEGfxGetWindowHeight();

			float textX = (room->rect.GetCenter().x - camX) * 2.0f / winW;
			float textY = (room->rect.GetCenter().y - camY) * 2.0f / winH;

			if (g_FontId >= 0 && textX > -0.9f && textX < 0.9f && textY > -0.9f && textY < 0.9f)
			{
				char buffer[] = "EXIT";
				AEGfxPrint(g_FontId, buffer, textX, textY, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
			}
		}
	}
	//draw item
	AEGfxVertexList* itemMesh = g_ItemsManager.GetItemMesh();
	if (itemMesh) {
		const auto& allItems = g_ItemsManager.GetItems();

		// Save current render mode and color state
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);

		for (const auto& item : allItems) {
			if (item.collected || !item.active) continue;

			// Check if item is in any discovered room
			bool shouldDraw = false;
			for (const auto& room : g_DungeonRooms) {
				if (room->isDiscovered) {
					// Check if item is inside this discovered room
					if (item.x >= room->rect.left && item.x <= room->rect.right &&
						item.y >= room->rect.bottom && item.y <= room->rect.top) {
						shouldDraw = true;
						break;
					}
				}
			}

			if (shouldDraw) {
				// Draw item at its position
				AEMtx33 scale, trans, transform;
				AEMtx33Scale(&scale, item.visualRadius * 100.0f, item.visualRadius * 100.0f);
				AEMtx33Trans(&trans, item.x, item.y);
				AEMtx33Concat(&transform, &trans, &scale);

				AEGfxSetColorToMultiply(item.color[0], item.color[1],
					item.color[2], item.color[3]);
				AEGfxSetTransform(transform.m);
				AEGfxMeshDraw(itemMesh, AE_GFX_MDM_TRIANGLES);
			}
		}

	}
	// Draw Character
	if (g_Character)
	{
		g_Character->Draw();

		// Draw Wayfinder Path
		if (g_ShowWayfinder && g_BossRoom)
		{
			// First locate which room the player is currently in
			Room* playerRoom = nullptr;
			float px = g_Character->GetWorldX();
			float py = g_Character->GetWorldY();

			for (const auto& room : g_DungeonRooms)
			{
				if (px >= room->rect.left && px <= room->rect.right &&
					py >= room->rect.bottom && py <= room->rect.top)
				{
					playerRoom = room.get();
					break;
				}
			}

			// Calculate path if player is inside a room
			if (playerRoom)
			{
				std::vector<AEVec2> path = GetPathToBoss(playerRoom, g_BossRoom);

				// Draw path segments
				if (!path.empty())
				{
					// Draw line from player to first room center
					DrawLineSegment(px, py, path[0].x, path[0].y, 5.0f, 1.0f, 1.0f, 0.0f, 0.5f);

					// Draw connections between subsequent room centers
					for (size_t i = 0; i < path.size() - 1; ++i)
					{
						DrawLineSegment(path[i].x, path[i].y, path[i + 1].x, path[i + 1].y, 5.0f, 1.0f, 1.0f, 0.0f, 0.5f);
					}
				}
			}
		}
	}

	g_Enemy.Draw();

}

// Cleans up memory
void Level_Free()
{
	for (auto& room : g_DungeonRooms)
	{
		if (room)
		{
			room->ClearNeighbours();
		}
	}
	g_DungeonRooms.clear();
	g_DungeonRooms.shrink_to_fit();

	g_Character = nullptr;
	g_BossRoom = nullptr;

	// Reset items
	g_ItemsInitialized = false;
}

// Unloads GPU meshes
void Level_Unload()
{
	if (g_pUnitSquare) { AEGfxMeshFree(g_pUnitSquare); g_pUnitSquare = nullptr; }
	if (g_pRectOutline) { AEGfxMeshFree(g_pRectOutline); g_pRectOutline = nullptr; }

	if (g_FontId >= 0) {
		AEGfxDestroyFont(g_FontId);
		g_FontId = -1;
	}

	// Release any textures or allocated data inside the player class

	g_Enemy.Unload();
}