#include "Level.h"
#include "SettingsMenu.h"
#include "RoomGenerator.h"
#include "jogo.h" 
#include "GameStateManager.h"
#include "Enemy.h"
#include "Items.h"
#include "AABBCollision.h"
#include "Tilesets.h" 
#include "Inventory.h"
#include "Leaderboard.h"
#include "AudioSystem.h"
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
static AEGfxTexture* g_texWall = nullptr;  //wall

// geometry pointers
static AEGfxVertexList* g_pUnitSquare = nullptr;
static AEGfxVertexList* g_pRectOutline = nullptr;

// ENTITY DATA

// player character
static std::unique_ptr<Character> g_Character = nullptr;

// enemies
static std::vector<SimpleEnemy> g_Enemies;
static int g_Difficulty = 1;
static int lives = 3;
static float invun_timer = 0.0f;

// --- JUICE & VISUAL EFFECTS ---
struct DamageParticle {
	float x, y, vx, vy, life, maxLife;
};
static std::vector<DamageParticle> g_DamageParticles;
static float g_ScreenShakeTimer = 0.0f;
static AEGfxTexture* g_pJumpscareTex = nullptr;

// END ENTITY DATA

// Tutorial State
static bool s_ShowTutorial = false;

// --- TUTORIAL TEXTURES ---
static AEGfxTexture* tut_EnokiTex = nullptr;
static AEGfxTexture* tut_EnemyTex = nullptr;
static AEGfxTexture* tut_BlueMushTex = nullptr;
static AEGfxTexture* tut_GreenMushTex = nullptr;
static AEGfxTexture* tut_RedMushTex = nullptr;
static AEGfxTexture* tut_KeyTex = nullptr;

// Level state
static bool g_RevealNeighbors = true;
static Room* g_BossRoom = nullptr;
static bool g_ShowWayfinder = false;
static bool g_ShowColors = false;
static bool g_ShowKeyLocation = false;
static s8 g_FontId = -1;

// Settings/pause state
static bool s_ShowSettings = false;

// Level transition state
static bool s_ShowLevelComplete = false;
static bool s_EnemyContact = false;

// End room UI state
static float g_EndRoomTextAlpha = 0.0f;
static bool g_EndRoomFadeIn = true;
static float g_FadeSpeed = 1.5f;

// Leaderboard state
static bool s_IsEnteringName = false;
static std::string s_CurrentName = "";
static bool s_ShowNameWarning = false;

// Debug State
static bool s_ShowDebug = false;
static bool s_GodMode = false;
static bool s_NoClip = false;

/**
 * @brief Draws a rotated and scaled line segment between two points using a unit square mesh.
 * @param x1 The starting X coordinate.
 * @param y1 The starting Y coordinate.
 * @param x2 The ending X coordinate.
 * @param y2 The ending Y coordinate.
 * @param thickness The width of the line segment.
 * @param r Red color value (0.0 to 1.0).
 * @param g Green color value (0.0 to 1.0).
 * @param b Blue color value (0.0 to 1.0).
 * @param a Alpha (transparency) value (0.0 to 1.0).
 */
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

/**
 * @brief Calculates the shortest path between two rooms using Breadth-First Search (BFS).
 * @param startRoom Pointer to the room where the search begins.
 * @param targetRoom Pointer to the destination room.
 * @return A vector of 2D coordinates representing the centers of the rooms along the path.
 * Returns an empty vector if no path is found.
 */
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

/**
 * @brief Checks if a specific tile is accessible from a starting tile using Breadth-First Search (BFS).
 * This prevents items, keys, and enemies from spawning inside closed-off wall structures generated by the procedural room templates. It ensures a valid walking path exists.
 * @param room    Pointer to the room containing the tilemap to check.
 * @param startX  The X grid coordinate to start the pathfinding from (usually 8, the room center).
 * @param startY  The Y grid coordinate to start the pathfinding from (usually 8, the room center).
 * @param targetX The target X grid coordinate to test for reachability.
 * @param targetY The target Y grid coordinate to test for reachability.
 * @return true   If a valid walking path exists from the start tile to the target tile.
 * @return false  If the target is a wall, out of bounds, or completely enclosed by walls.
 */
static bool IsTileReachable(Room* room, int startX, int startY, int targetX, int targetY)
{
	// If the target is a wall, it's obviously unreachable
	if (room->tileMap[targetY][targetX] != 0) return false;

	std::queue<std::pair<int, int>> frontier;
	bool visited[16][16] = { false };

	frontier.push({ startX, startY });
	visited[startY][startX] = true;

	int dx[] = { 0, 0, 1, -1 };
	int dy[] = { 1, -1, 0, 0 };

	while (!frontier.empty())
	{
		auto curr = frontier.front();
		frontier.pop();

		if (curr.first == targetX && curr.second == targetY) return true;

		for (int i = 0; i < 4; ++i)
		{
			int nx = curr.first + dx[i];
			int ny = curr.second + dy[i];

			// Check bounds and ensure it's a floor tile (0)
			if (nx >= 0 && nx < 16 && ny >= 0 && ny < 16)
			{
				if (room->tileMap[ny][nx] == 0 && !visited[ny][nx])
				{
					visited[ny][nx] = true;
					frontier.push({ nx, ny });
				}
			}
		}
	}
	return false; // Completely blocked off
}

void Level_Load()
{
	if (g_pUnitSquare == nullptr) {
		AEGfxMeshStart();

		// Triangle 1
		AEGfxTriAdd(
			-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
			0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
			-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

		// Triangle 2
		AEGfxTriAdd(
			0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
			0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
			-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

		g_pUnitSquare = AEGfxMeshEnd();
	}

	if (g_pRectOutline == nullptr) {
		AEGfxMeshStart();

		AEGfxVertexAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
		AEGfxVertexAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f);
		AEGfxVertexAdd(0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f);
		AEGfxVertexAdd(-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
		AEGfxVertexAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);

		g_pRectOutline = AEGfxMeshEnd();
	}
	if (g_FontId < 0) {
		g_FontId = AEGfxCreateFont("Assets/exo2-regular.ttf", 20);
		if (g_FontId < 0) g_FontId = AEGfxCreateFont("Assets\\exo2-regular.ttf", 20);
	}

	// wall texture
	if (g_texWall == nullptr) g_texWall = AEGfxTextureLoad("Assets/stone-texture-background.jpg");
	// --- LOAD TUTORIAL SPRITES ---
	if (tut_EnokiTex == nullptr) tut_EnokiTex = AEGfxTextureLoad("Assets/jogo.png");
	if (tut_EnemyTex == nullptr) tut_EnemyTex = AEGfxTextureLoad("Assets/enemy.png");
	if (tut_BlueMushTex == nullptr) tut_BlueMushTex = AEGfxTextureLoad("Assets/bluemushroom.png"); // Check your filename!
	if (tut_GreenMushTex == nullptr) tut_GreenMushTex = AEGfxTextureLoad("Assets/greenmushroom.png"); // Check your filename!
	if (tut_RedMushTex == nullptr) tut_RedMushTex = AEGfxTextureLoad("Assets/redmushroom.png"); // Check your filename!
	if (tut_KeyTex == nullptr) tut_KeyTex = AEGfxTextureLoad("Assets/babycarrot.png");
	TilesetManager::Load();

	// Initialize items graphics (AFTER engine is ready)
	g_ItemsManager.InitializeGraphics();
	g_Inventory.Load();
	// Jumpscare img
	g_pJumpscareTex = AEGfxTextureLoad("Assets/enemy.png");
	// Load shared settings menu resources
	SettingsMenu_Load();

	// Reset all Debug and Tool flags to FALSE
	s_ShowDebug = false; // The menu itself starts hidden
	s_GodMode = false;
	s_NoClip = false;
	g_RevealNeighbors = true;
	g_ShowWayfinder = false;
	g_ShowColors = false;
	g_ShowKeyLocation = false;
}

void Level_Init()
{
	// If just came from the Main Menu or hitting Restart, reset the timer
	if (gGameStatePrev == GS_MAINMENU || gGameStateCurr == GS_RESTART) {
		g_RunTimer = 0.0f;
	}

	if (gGameStateCurr == GS_RESTART) {
		g_ItemsInitialized = false;  // Force fresh spawn
		g_ItemsManager.Clear();       // Clear old items (you need to add this method)
		printf("=== RESTARTING - Items will be respawned ===\n");
	}

	// Calculate current level mathematically
	g_CurrentRunLevel = (gGameStateCurr - GS_LEVEL1) + 1;
	if (gGameStateCurr == GS_LEVEL1) {
		lives = 3;
		s_ShowTutorial = true; // --- SHOW TUTORIAL ON LEVEL 1 ---
	}
	else {
		s_ShowTutorial = false; // Hide it on deeper levels
	}
	// Damage Particles clear
	g_DamageParticles.clear();
	g_ScreenShakeTimer = 0.0f;

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

		// Calculate shrink factor based on current level
		int currentLevelNum = 1;
		if (gGameStateCurr == GS_LEVEL2) currentLevelNum = 2;
		else if (gGameStateCurr == GS_LEVEL3) currentLevelNum = 3;
		else if (gGameStateCurr == GS_LEVEL4) currentLevelNum = 4;
		else if (gGameStateCurr == GS_LEVEL5) currentLevelNum = 5;
		else if (gGameStateCurr == GS_LEVEL6) currentLevelNum = 6;

		// Start at 1.0 (100%), and shrink by 10% (0.1f) for every level completed
		float shrinkFactor = 1.0f - ((currentLevelNum - 1) * 0.1f);

		// Safety Net: Cap the maximum shrinkage so the game doesn't become literally unplayable!
		if (shrinkFactor < 0.4f) shrinkFactor = 0.4f;

		// Apply the shrinkage to the player BEFORE loading the mesh
		g_Character->visionMultiplier = shrinkFactor;

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

	int totalEnemiesToSpawn = Random::Range(1, 4);
	int spawnedCount = 0;

	while (spawnedCount < totalEnemiesToSpawn)
	{
		int roomIdx = Random::Range(0, (int)g_DungeonRooms.size() - 1);
		auto& room = g_DungeonRooms[roomIdx];

		if (room->type == RoomType::Start) continue;

		int randCol = Random::Range(1, 14);
		int randRow = Random::Range(1, 14);

		// Use IsTileReachable so enemies don't spawn trapped inside boxes
		if (IsTileReachable(room.get(), 8, 8, randCol, randRow))
		{
			float spawnX = room->rect.left + (randCol * room->tileSize) + (room->tileSize * 0.5f);
			float spawnY = room->rect.top - (randRow * room->tileSize) - (room->tileSize * 0.5f);

			SimpleEnemy newEnemy;
			newEnemy.Load();
			newEnemy.SetPosition(spawnX, spawnY);

			int diceRoll = Random::Range(1, 6);
			newEnemy.SetChaseDuration(10.0f + static_cast<float>(diceRoll) * 5.0f);
			newEnemy.currentState = EnemyState::PATROL;

			g_Enemies.push_back(newEnemy);
			spawnedCount++;
		}
	}
	s_EnemyContact = false;

	if (!g_ItemsInitialized)
	{

		// CRITICAL: Clear old items before spawning new ones
		g_ItemsManager.Clear();
		printf("=== Clearing old items before spawning fresh ===\n");

		// --- SPAWN REGULAR ITEMS ---
		for (const auto& room : g_DungeonRooms)
		{
			if (room->type == RoomType::Start || room->type == RoomType::Boss) continue;

			if (std::rand() % 100 < 30)
			{
				int randomType = std::rand() % 3;
				int rx = 8, ry = 8; // Default to room center

				// Try to find a safely reachable tile instead of always spawning at center
				for (int i = 0; i < 50; i++) {
					int tx = std::rand() % 16;
					int ty = std::rand() % 16;
					if (IsTileReachable(room.get(), 8, 8, tx, ty)) {
						rx = tx; ry = ty; break;
					}
				}

				float itemX = room->rect.left + (rx * room->tileSize) + (room->tileSize * 0.5f);
				float itemY = room->rect.top - (ry * room->tileSize) - (room->tileSize * 0.5f);
				g_ItemsManager.SpawnItem(itemX, itemY, (ItemType)randomType);
			}
		}

		// --- SPAWN 3 KEYS in random rooms (which does not include start and boss/end room) ---
		std::vector<Room*> eligibleRooms;
		for (const auto& room : g_DungeonRooms)
		{
			if (room->type != RoomType::Start && room->type != RoomType::Boss)
			{
				eligibleRooms.push_back(room.get());
			}
		}

		// Spawn 3 keys (they will all stack in slot 4 when collected)
		int keysSpawned = 0;
		int maxKeys = 3;

		while (keysSpawned < maxKeys && !eligibleRooms.empty())
		{
			// Pick a random room
			int randomRoomIndex = std::rand() % eligibleRooms.size();
			Room* keyRoom = eligibleRooms[randomRoomIndex];

			int attempts = 0;
			bool keyPlaced = false;

			while (!keyPlaced && attempts < 100)
			{
				int tileX = std::rand() % 16;
				int tileY = std::rand() % 16;

				// Verify the key is physically reachable from the room center
				if (IsTileReachable(keyRoom, 8, 8, tileX, tileY))
				{
					float keyX = keyRoom->rect.left + (tileX * keyRoom->tileSize) + (keyRoom->tileSize * 0.5f);
					float keyY = keyRoom->rect.top - (tileY * keyRoom->tileSize) - (keyRoom->tileSize * 0.5f);

					g_ItemsManager.SpawnItem(keyX, keyY, ItemType::KEY);
					keyPlaced = true;
					keysSpawned++;
					printf("Key %d spawned safely in room at (%f, %f)\n", keysSpawned, keyX, keyY);
				}
				attempts++;
			}

			if (!keyPlaced)
			{
				// Fallback: place at room center
				g_ItemsManager.SpawnItem(keyRoom->rect.GetCenter().x, keyRoom->rect.GetCenter().y, ItemType::KEY);
				keysSpawned++;
				printf("Key %d spawned at room center (Fallback)\n", keysSpawned);
			}
			// prevents spawning of multiple keys in same room
			eligibleRooms.erase(eligibleRooms.begin() + randomRoomIndex);
		}

		printf("Total keys spawned: %d\n", keysSpawned);

		g_ItemsInitialized = true;
	}

	g_Inventory.Init();

	s_ShowLevelComplete = false;
	s_ShowSettings = false;

	s_IsEnteringName = false;
	s_CurrentName = "";
	s_ShowNameWarning = false;

	SettingsMenu_Initialize();

	// Start the dungeon music
	AudioSystem::Play("LevelBGM");
}

void Level_Update()
{
	// Toggle Settings/Pause
	if (AEInputCheckTriggered(AEVK_ESCAPE) && !s_IsEnteringName && !s_ShowLevelComplete && lives > 0 && !s_ShowTutorial)
	{
		s_ShowSettings = !s_ShowSettings;
		printf("Settings Menu: %s\n", s_ShowSettings ? "ON" : "OFF");
	}

	if (s_ShowSettings)
	{
		SettingsMenu_Update(s_ShowSettings);
		return; // Stop game logic while menu is open
	}

	// Capture Player Name for Leaderboard
	if (s_IsEnteringName) {
		// Capture A-Z keys
		for (int i = AEVK_A; i <= AEVK_Z; ++i) {
			if (AEInputCheckTriggered(i)) {
				if (s_CurrentName.length() < 8) {
					s_CurrentName += (char)i;
					s_ShowNameWarning = false;
				}
				else {
					s_ShowNameWarning = true; // Tried to type more than 8!
				}
			}
		}
		// Capture Backspace to delete
		if (AEInputCheckTriggered(AEVK_BACK)) {
			if (!s_CurrentName.empty()) s_CurrentName.pop_back();
			s_ShowNameWarning = false;
		}
		// Press Enter to Save and Exit
		if (AEInputCheckTriggered(AEVK_RETURN) && !s_CurrentName.empty()) {
			LeaderboardSystem::AddRun(s_CurrentName, g_CurrentRunLevel, g_RunTimer);
			gGameStateNext = GS_MAINMENU;
		}
		return; // Stop the rest of the game from updating!
	}

	// Handle Level Complete Menu
	if (s_ShowLevelComplete) {
		if (AEInputCheckTriggered(AEVK_RETURN)) {
			if (gGameStateCurr < GS_LEVEL6) {
				gGameStateNext = (GAME_STATE)(gGameStateCurr + 1);
			}
			else {
				// Beat the game! Trigger Name Entry
				s_IsEnteringName = true;
			}
		}
		if (AEInputCheckTriggered(AEVK_BACK)) {
			// Trigger Name entry instead of instantly quitting!
			if (g_CurrentRunLevel > 1 || g_RunTimer > 10.0f) {
				s_IsEnteringName = true;
			}
			else {
				gGameStateNext = GS_MAINMENU; // Ignore short spam runs
			}
		}
		return;
	}

	// Handle Tutorial Overlay
	if (s_ShowTutorial) {
		if (AEInputCheckTriggered(AEVK_RETURN)) {
			s_ShowTutorial = false; // Dismiss tutorial
		}
		return; // Halt the update loop so the game stays paused!
	}

	// Check if player is dead before updating physics or timers
	if (lives <= 0) {
		if (AEInputCheckTriggered(AEVK_BACK)) {
			gGameStateNext = GS_MAINMENU; // Return to menu
		}
		if (AEInputCheckTriggered(AEVK_R)) {
			gGameStateNext = GS_RESTART; // Allow quick restart
		}
		return; // Stop the rest of the game from running!
	}

	float dt = (float)AEFrameRateControllerGetFrameTime();
	g_RunTimer += dt;

	// Normal Level Updates
	if (AEInputCheckTriggered(AEVK_R)) { gGameStateNext = GS_RESTART; printf("Restarting Level...\n"); }

	// --- F3 DEBUG MENU CONTROLS ---
	if (AEInputCheckTriggered(AEVK_F3)) {
		s_ShowDebug = !s_ShowDebug;
	}

	// Only allow these toggles if the Debug Menu is OPEN!
	if (s_ShowDebug) {
		if (AEInputCheckTriggered(AEVK_G)) { s_GodMode = !s_GodMode; printf("Godmode: %s\n", s_GodMode ? "ON" : "OFF"); }
		if (AEInputCheckTriggered(AEVK_V)) {
			s_NoClip = !s_NoClip;
			if (g_Character) g_Character->isNoClip = s_NoClip; // Tell the player to ignore walls!
			printf("Noclip: %s\n", s_NoClip ? "ON" : "OFF");
		}

		// Map & Visual Debuggers
		if (AEInputCheckTriggered(AEVK_N)) { g_RevealNeighbors = !g_RevealNeighbors; printf("Reveal Neighbors: %s\n", g_RevealNeighbors ? "ON" : "OFF"); }
		if (AEInputCheckTriggered(AEVK_M)) { g_ShowWayfinder = !g_ShowWayfinder; printf("Show Wayfinder: %s\n", g_ShowWayfinder ? "ON" : "OFF"); }
		if (AEInputCheckTriggered(AEVK_C)) { g_ShowColors = !g_ShowColors; printf("Show Colors: %s\n", g_ShowColors ? "ON" : "OFF"); }
		if (AEInputCheckTriggered(AEVK_K)) { g_ShowKeyLocation = !g_ShowKeyLocation; printf("Key Tracker: %s\n", g_ShowKeyLocation ? "ON" : "OFF"); }
	}

	// Update inventory (handles number key presses)
	g_Inventory.Update();

	if (g_Character)
	{
		// Check Exit Door Collision
		if (g_BossRoom) {
			float px = g_Character->GetWorldX();
			float py = g_Character->GetWorldY();

			// Check if player is inside the Boss/Exit Room boundaries
			if (px >= g_BossRoom->rect.left && px <= g_BossRoom->rect.right &&
				py >= g_BossRoom->rect.bottom && py <= g_BossRoom->rect.top)
			{
				// Update fade timer for text pulse effect
				if (g_EndRoomFadeIn)
				{
					g_EndRoomTextAlpha += g_FadeSpeed * dt;
					if (g_EndRoomTextAlpha >= 1.0f) { g_EndRoomTextAlpha = 1.0f; g_EndRoomFadeIn = false; }
				}
				else
				{
					g_EndRoomTextAlpha -= g_FadeSpeed * dt;
					if (g_EndRoomTextAlpha <= 0.2f) { g_EndRoomTextAlpha = 0.2f; g_EndRoomFadeIn = true; }
				}

				if (AEInputCheckTriggered(AEVK_E)) {
					int keyCount = g_Inventory.GetKeyCount();
					if (keyCount >= 3) {
						printf("Door Unlocked! You have all 3 keys!\n");
						s_ShowLevelComplete = true; // Pause game and show the menu
					}
					else {
						printf("The door is locked. You need %d more key%s! (Slot 4: %d/3)\n",
							3 - keyCount, (3 - keyCount == 1) ? "" : "s", keyCount);
					}
				}
			}
			else
			{
				// Reset alpha when player leaves the room
				g_EndRoomTextAlpha = 0.0f;
				g_EndRoomFadeIn = true;
			}
		}

		g_Character->Update(g_DungeonRooms);
		g_Character->UpdateAbilities(dt, g_Enemies, g_ItemsManager, g_DungeonRooms);
		g_Character->CheckItemCollection(g_ItemsManager);

		// --- UPDATE DAMAGE PARTICLES ---
		for (auto it = g_DamageParticles.begin(); it != g_DamageParticles.end(); ) {
			it->x += it->vx * dt;
			it->y += it->vy * dt;
			it->life -= dt;
			if (it->life <= 0.0f) it = g_DamageParticles.erase(it);
			else ++it;
		}

		// --- UPDATE CAMERA WITH SCREEN SHAKE ---
		float camX = g_Character->GetWorldX();
		float camY = g_Character->GetWorldY();

		if (g_ScreenShakeTimer > 0.0f) {
			g_ScreenShakeTimer -= dt;
			// Shake the camera wildly by up to 15 pixels in any direction
			camX += (float)((rand() % 30) - 15);
			camY += (float)((rand() % 30) - 15);
		}
		AEGfxSetCamPosition(camX, camY);

		// Room Discovery
		for (auto& room : g_DungeonRooms)
		{
			if (Collision_PointInRect(g_Character->GetWorldX(), g_Character->GetWorldY(), room->rect))
			{
				room->isDiscovered = true;
				if (g_RevealNeighbors)
				{
					for (auto* neighbor : room->GetNeighbours()) if (neighbor) neighbor->isDiscovered = true;
				}
				break;
			}
		}

		// Update Enemies and Items
		float playerWorldX = g_Character->GetWorldX();
		float playerWorldY = g_Character->GetWorldY();

		for (auto& enemy : g_Enemies)
		{
			enemy.Update(playerWorldX, playerWorldY, dt, g_DungeonRooms);
			float dx = playerWorldX - enemy.GetWorldX();
			float dy = playerWorldY - enemy.GetWorldY();
			float distToPlayer = dx * dx + dy * dy;
			// 625 = 25^2
			if (distToPlayer < 625 && !s_EnemyContact && !enemy.IsStunned() && !s_GodMode) {
				AudioSystem::Play("Hit");
				s_EnemyContact = true;
				lives--;
				g_Character->TriggerStun(g_Enemies);

				// --- TRIGGER IMPACT JUICE ---
				g_ScreenShakeTimer = 0.35f; // 0.35 seconds of violent shake

				// Spawn 40 blood/spark particles bursting outward
				for (int i = 0; i < 40; ++i) {
					float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
					float speed = (float)((rand() % 400) + 150); // Explode outwards fast
					g_DamageParticles.push_back({
						playerWorldX, playerWorldY,
						cosf(angle) * speed, sinf(angle) * speed,
						0.4f, 0.4f // Live for 0.4 seconds
						});
				}
			}
		}

		if (s_EnemyContact) {
			invun_timer += dt;
			if (invun_timer >= 2.0f) {
				s_EnemyContact = false;
				invun_timer = 0.0f;
			}
		}

		g_ItemsManager.Update(playerWorldX, playerWorldY, 0.0f);
	}

	// Debug: Show item count when 'I' is pressed
	if (AEInputCheckTriggered(AEVK_I))
	{
		printf("Items collected: %d/%d\n",
			g_ItemsManager.GetCollectedCount(),
			g_ItemsManager.GetTotalCount());
	}
}

void Level_Draw()
{
	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);

	// Logic for drawing rooms.
	for (const auto& room : g_DungeonRooms)
	{
		// Skips drawing if the room has not been discovered by the player.
		if (!room->isDiscovered) continue;
		// Retrieves the visual style data for the current room.
		const TilesetData& style = TilesetManager::Get(room->tilesetID);

		// Resets the color to white so the previous room's wall color (black) 
		// doesn't make this room's floor black.
		AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

		// Sets up the transformation matrix for the floor mesh.
		AEMtx33 scale, trans, transform;
		AEMtx33Scale(&scale, (float)room->rect.width(), (float)room->rect.height());
		AEMtx33Trans(&trans, room->rect.GetCenter().x, room->rect.GetCenter().y);
		AEMtx33Concat(&transform, &trans, &scale);

		// --- DRAW FLOOR (Reverted to colors) ---
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxTextureSet(nullptr, 0, 0);

		if (room->type == RoomType::Boss) AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f);
		else if (!g_ShowColors) AEGfxSetColorToMultiply(0.5f, 0.5f, 0.5f, 1.0f);
		else if (room->type == RoomType::Start)
		{
			AEGfxSetColorToMultiply(0.0f, 1.0f, 0.0f, 1.0f);
		}
		else if(room->type == RoomType::Boss) {
			// Boss Room is always a unique color to identify it
			AEGfxSetRenderMode(AE_GFX_RM_COLOR);
			AEGfxSetColorToMultiply(0.8f, 0.0f, 0.0f, 1.0f); // Vibrant Red
		}
		else if (!g_ShowColors) {
			// BASE MODE: Uniform Grayscale for standard rooms
			AEGfxSetRenderMode(AE_GFX_RM_COLOR);
			AEGfxSetColorToMultiply(0.3f, 0.3f, 0.3f, 1.0f); // Medium Gray
		}
		else {
			// DEBUG MODE: Use the specific tileset color (Purple, Blue, etc.)
			AEGfxSetRenderMode(AE_GFX_RM_COLOR);
			AEGfxSetColorToMultiply(style.r, style.g, style.b, 1.0f);
		}

		AEGfxSetTransform(transform.m);

		// Executes the draw call for the floor quad.
		AEGfxMeshDraw(g_pUnitSquare, AE_GFX_MDM_TRIANGLES);

		// --- DRAW WALLS ---
		AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
		AEGfxTextureSet(g_texWall, 0, 0);
		AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f); // Pure true-color texture for walls

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

		AEGfxTextureSet(nullptr, 0, 0);

		// Renders the exit text specifically for the boss room.
		if (room->type == RoomType::Boss)
		{
			// Temporarily disable textures for text printing
			AEGfxTextureSet(nullptr, 0, 0);
			AEGfxSetRenderMode(AE_GFX_RM_COLOR);

			float camX = g_Character ? g_Character->GetWorldX() : 0.0f;
			float camY = g_Character ? g_Character->GetWorldY() : 0.0f;
			float textX = (room->rect.GetCenter().x - camX) * 2.0f / AEGfxGetWindowWidth();
			float textY = (room->rect.GetCenter().y - camY) * 2.0f / AEGfxGetWindowHeight();
			if (g_FontId >= 0 && textX > -0.9f && textX < 0.9f && textY > -0.9f && textY < 0.9f)
				AEGfxPrint(g_FontId, (char*)"exit", textX, textY, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		}

		// Reset back to standard color rendering for the rest of the game loop
		AEGfxTextureSet(nullptr, 0, 0);
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	}

	// Draw Items
	// Draw Items (Now uses the official textured Draw function in Items.cpp!)
	g_ItemsManager.Draw();
	// --- DRAW ENEMY VISION CONES FIRST ---
	AEGfxSetBlendMode(AE_GFX_BM_BLEND); // Ensure blending is ON for the cones
	for (auto& enemy : g_Enemies)
	{
		enemy.DrawVisionOverlay();
	}

	// --- DRAW ENEMY SPRITES SECOND ---
	for (auto& enemy : g_Enemies)
	{
		enemy.Draw(); // This function handles its own internal states
	}
	// --- DRAW THE DARKNESS OVERLAY HERE ---
	if (g_Character) {
		g_Character->DrawVisionOverlay();

		// Draw the Player and Abilities LAST so they sit on top of the darkness
		g_Character->DrawAbilities();
		g_Character->Draw();
	}
	// --- DRAW DAMAGE PARTICLES ---
	if (!g_DamageParticles.empty()) {
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		for (const auto& p : g_DamageParticles) {
			float alpha = p.life / p.maxLife; // Fade out as they die
			AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, alpha); // Red Blood

			AEMtx33 scale, trans, transform;
			AEMtx33Scale(&scale, 8.0f, 8.0f); // Small 8x8 pixel chunks
			AEMtx33Trans(&trans, p.x, p.y);
			AEMtx33Concat(&transform, &trans, &scale);

			AEGfxSetTransform(transform.m);
			AEGfxMeshDraw(g_pUnitSquare, AE_GFX_MDM_TRIANGLES);
		}
		AEGfxSetBlendMode(AE_GFX_BM_NONE);
	}
	// -------------------------------------------

	// Wayfinder to boss room
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

	// Wayfinder to key location
	if (g_ShowKeyLocation && g_Character)
	{
		float pX = g_Character->GetWorldX();
		float pY = g_Character->GetWorldY();

		Item* closestKey = nullptr;

		// Initialize with a massive number so keys at ANY distance are valid
		float minDistanceSq = 1e12f;

		// Scan EVERY item in the manager
		for (auto& item : g_ItemsManager.GetItems())
		{
			// Must be a KEY and must NOT be collected yet
			if (item.type == ItemType::KEY && !item.collected)
			{
				float dx = item.x - pX;
				float dy = item.y - pY;

				// Squared distance check
				float distSq = (dx * dx) + (dy * dy);

				if (distSq < minDistanceSq)
				{
					minDistanceSq = distSq;
					closestKey = const_cast<Item*>(&item);
				}
			}
		}

		// After checking the WHOLE map, if a key exists, draw it
		if (closestKey)
		{
			// This line will now stretch across the entire map if needed
			DrawLineSegment(
				pX, pY,
				closestKey->x, closestKey->y,
				5.0f, 1.0f, 0.0f, 1.0f, 0.7f // Purple
			);

			// Draw the highlight box at the key's world coordinates
			AEMtx33 scale, trans, transform;
			AEMtx33Scale(&scale, 120.0f, 120.0f);
			AEMtx33Trans(&trans, closestKey->x, closestKey->y);
			AEMtx33Concat(&transform, &trans, &scale);

			AEGfxSetRenderMode(AE_GFX_RM_COLOR);
			AEGfxSetColorToMultiply(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
			AEGfxSetTransform(transform.m);

			if (g_pRectOutline) {
				AEGfxMeshDraw(g_pRectOutline, AE_GFX_MDM_LINES_STRIP);
			}
		}
	}

	// Draw End Room Fading Reminders
	if (g_Character && g_BossRoom && g_EndRoomTextAlpha > 0.0f)
	{
		float px = g_Character->GetWorldX();
		float py = g_Character->GetWorldY();

		if (px >= g_BossRoom->rect.left && px <= g_BossRoom->rect.right &&
			py >= g_BossRoom->rect.bottom && py <= g_BossRoom->rect.top)
		{
			int keyCount = g_Inventory.GetKeyCount();
			char reminderBuf[64];

			if (keyCount >= 3)
			{
				sprintf_s(reminderBuf, "PRESS 'E' TO ESCAPE!");
			}
			else
			{
				sprintf_s(reminderBuf, "COLLECT ALL KEYS (%d/3)", keyCount);
			}

			if (g_FontId >= 0)
			{
				// Position text near the center bottom
				AEGfxPrint(g_FontId, reminderBuf, -0.22f, -0.6f, 1.2f, 1.0f, 1.0f, 1.0f, g_EndRoomTextAlpha);
			}
		}
	}

	// DRAW THE RUN TIMER
	if (g_FontId >= 0) {
		// DRAW THE RUN TIMER
		if (g_FontId >= 0) {
			int minutes = static_cast<int>(g_RunTimer) / 60;
			float seconds = fmod(g_RunTimer, 60.0f);
			char timeBuf[32];
			sprintf_s(timeBuf, "Time: %02d:%05.2f", minutes, seconds);

			// FIX: Bumped Y up to 0.90f!
			AEGfxPrint(g_FontId, timeBuf, -0.95f, 0.90f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
	//DEBUG MENU
	if (s_ShowDebug && g_FontId >= 0) {
		char fpsBuf[32], godBuf[32], clipBuf[32];
		char neighBuf[32], wayBuf[32], colBuf[32], keyBuf[32];

		// Format all the strings
		sprintf_s(fpsBuf, "FPS: %.0f", (float)AEFrameRateControllerGetFrameRate());
		sprintf_s(godBuf, "Godmode (G): %s", s_GodMode ? "ON" : "OFF");
		sprintf_s(clipBuf, "Noclip (V): %s", s_NoClip ? "ON" : "OFF");
		sprintf_s(neighBuf, "Neighbors (N): %s", g_RevealNeighbors ? "ON" : "OFF");
		sprintf_s(wayBuf, "Wayfinder (M): %s", g_ShowWayfinder ? "ON" : "OFF");
		sprintf_s(colBuf, "Colors (C): %s", g_ShowColors ? "ON" : "OFF");
		sprintf_s(keyBuf, "Key Tracker (K): %s", g_ShowKeyLocation ? "ON" : "OFF");

		// Stack them neatly down the left side of the screen
		AEGfxPrint(g_FontId, fpsBuf, -0.95f, 0.65f, 0.8f, 0.0f, 1.0f, 0.0f, 1.0f); // Green
		AEGfxPrint(g_FontId, godBuf, -0.95f, 0.60f, 0.8f, 0.0f, 1.0f, 0.0f, 1.0f);
		AEGfxPrint(g_FontId, clipBuf, -0.95f, 0.55f, 0.8f, 0.0f, 1.0f, 0.0f, 1.0f);
		AEGfxPrint(g_FontId, neighBuf, -0.95f, 0.50f, 0.8f, 0.0f, 1.0f, 0.0f, 1.0f);
		AEGfxPrint(g_FontId, wayBuf, -0.95f, 0.45f, 0.8f, 0.0f, 1.0f, 0.0f, 1.0f);
		AEGfxPrint(g_FontId, colBuf, -0.95f, 0.40f, 0.8f, 0.0f, 1.0f, 0.0f, 1.0f);
		AEGfxPrint(g_FontId, keyBuf, -0.95f, 0.35f, 0.8f, 0.0f, 1.0f, 0.0f, 1.0f);
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

	//draw lives and keys
	if (g_FontId >= 0)
	{
		char livesText[16];
		// Formats the string: "Lives: 3"
		sprintf_s(livesText, "Lives: %d", lives);

		// AEGfxPrint(fontId, string, x, y, scale, r, g, b, a)
		// x = 0.7f (Right side), y = 0.9f (Top)
		AEGfxPrint(g_FontId, livesText, 0.7f, 0.9f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f);

		// --- NEW: DRAW KEYS ---
		char keyText[32];
		int collectedKeys = g_ItemsManager.GetCollectedKeyCount();
		int totalKeys = g_ItemsManager.GetTotalKeyCount();
		sprintf_s(keyText, "Keys: %d/%d", collectedKeys, totalKeys);

		// Print slightly below the lives (y = 0.8f) in Yellow (R:1.0f, G:1.0f, B:0.0f)
		AEGfxPrint(g_FontId, keyText, 0.7f, 0.8f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f);
		// ----------------------
	}

	if (s_ShowLevelComplete)
	{
		// Draw a semi-transparent dark background
		float camX = g_Character ? g_Character->GetWorldX() : 0.0f;
		float camY = g_Character ? g_Character->GetWorldY() : 0.0f;

		AEMtx33 scale, trans, transform;
		AEMtx33Scale(&scale, 4000.0f, 4000.0f); // Make it huge to cover the screen
		AEMtx33Trans(&trans, camX, camY);
		AEMtx33Concat(&transform, &trans, &scale);

		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.85f); // 85% opacity black
		AEGfxSetTransform(transform.m);
		AEGfxMeshDraw(g_pUnitSquare, AE_GFX_MDM_TRIANGLES);
		AEGfxSetBlendMode(AE_GFX_BM_NONE);

		// Draw the Text (Using NDC coordinates so it sticks to the screen)
		if (g_FontId >= 0) {
			AEGfxPrint(g_FontId, (char*)"LEVEL COMPLETE!", -0.2f, 0.3f, 1.5f, 0.0f, 1.0f, 0.0f, 1.0f); // Green text

			if (gGameStateCurr < GS_LEVEL6) {
				AEGfxPrint(g_FontId, (char*)"Press [ENTER] to Descend Deeper", -0.235f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
			}
			else {
				AEGfxPrint(g_FontId, (char*)"YOU ESCAPED THE DUNGEON!", -0.26f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f);
			}

			AEGfxPrint(g_FontId, (char*)"Press [BACKSPACE] to Return to Menu", -0.26f, -0.2f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	// --- DRAW NAME ENTRY OVERLAY ---
	if (s_IsEnteringName) {
		// Draw dark background
		float camX = g_Character ? g_Character->GetWorldX() : 0.0f;
		float camY = g_Character ? g_Character->GetWorldY() : 0.0f;
		AEMtx33 scale, trans, transform;
		AEMtx33Scale(&scale, 4000.0f, 4000.0f);
		AEMtx33Trans(&trans, camX, camY);
		AEMtx33Concat(&transform, &trans, &scale);

		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.95f); // Super dark
		AEGfxSetTransform(transform.m);
		AEGfxMeshDraw(g_pUnitSquare, AE_GFX_MDM_TRIANGLES);
		AEGfxSetBlendMode(AE_GFX_BM_NONE);

		if (g_FontId >= 0) {
			AEGfxPrint(g_FontId, (char*)"RUN OVER!", -0.148f, 0.4f, 1.5f, 1.0f, 1.0f, 1.0f, 1.0f);
			AEGfxPrint(g_FontId, (char*)"Enter Username (Max 8 Chars)", -0.22f, 0.1f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

			// Draw the typed name!
			std::string displayStr = "> " + s_CurrentName + " <";
			AEGfxPrint(g_FontId, (char*)displayStr.c_str(), -0.15f, -0.1f, 1.2f, 1.0f, 1.0f, 0.0f, 1.0f); // Yellow

			if (s_ShowNameWarning) {
				AEGfxPrint(g_FontId, (char*)"Maximum 8 characters reached!", -0.2f, -0.25f, 0.8f, 1.0f, 0.0f, 0.0f, 1.0f); // Red
			}

			if (!s_CurrentName.empty()) {
				AEGfxPrint(g_FontId, (char*)"Press [ENTER] to Save Run", -0.2f, -0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f); // Green
			}
		}
	}

	if (lives <= 0)
	{
		float camX = g_Character ? g_Character->GetWorldX() : 0.0f;
		float camY = g_Character ? g_Character->GetWorldY() : 0.0f;

		// JUMPSCARE SHAKE: Violently shake the camera forever while dead
		camX += (float)((rand() % 60) - 30);
		camY += (float)((rand() % 60) - 30);

		// 1. Draw Pitch Black Background
		AEMtx33 scale, trans, transform;
		AEMtx33Scale(&scale, 4000.0f, 4000.0f);
		AEMtx33Trans(&trans, camX, camY);
		AEMtx33Concat(&transform, &trans, &scale);

		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f); // Solid Black
		AEGfxSetTransform(transform.m);
		AEGfxMeshDraw(g_pUnitSquare, AE_GFX_MDM_TRIANGLES);

		// 2. Draw Giant Red Jumpscare Face!
		if (g_pJumpscareTex) {
			AEMtx33Scale(&scale, 1500.0f, 1500.0f); // Massive Scale!
			AEMtx33Trans(&trans, camX, camY);
			AEMtx33Concat(&transform, &trans, &scale);

			AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
			AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f); // Tint it bloody red
			AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
			AEGfxTextureSet(g_pJumpscareTex, 0.0f, 0.0f);

			AEGfxSetTransform(transform.m);
			AEGfxMeshDraw(g_pUnitSquare, AE_GFX_MDM_TRIANGLES);
		}

		// 3. Draw "You Died" Text
		if (g_FontId >= 0) {
			AEGfxPrint(g_FontId, (char*)"YOU DIED", -0.1f, 0.3f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f);
			AEGfxPrint(g_FontId, (char*)"Press [BACKSPACE] to Return to Menu", -0.28f, -0.4f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		}

		// Reset Engine State
		AEGfxTextureSet(nullptr, 0.0f, 0.0f);
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetBlendMode(AE_GFX_BM_NONE);
	}
	// --- DRAW TUTORIAL OVERLAY ---
	if (s_ShowTutorial) {
		// Draw a semi-transparent dark background
		float camX = g_Character ? g_Character->GetWorldX() : 0.0f;
		float camY = g_Character ? g_Character->GetWorldY() : 0.0f;

		AEMtx33 scale, trans, transform;
		AEMtx33Scale(&scale, 4000.0f, 4000.0f);
		AEMtx33Trans(&trans, camX, camY);
		AEMtx33Concat(&transform, &trans, &scale);

		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.90f); // 90% opacity black
		AEGfxSetTransform(transform.m);
		AEGfxMeshDraw(g_pUnitSquare, AE_GFX_MDM_TRIANGLES);

		// Helper function to draw UI Sprites locked to the screen
		auto DrawUISprite = [&](AEGfxTexture* tex, float ndcX, float ndcY, float size) {
			if (!tex) return;

			// CRITICAL FIX: Use Engine World Bounds instead of raw pixels!
			// This perfectly synchronizes with AEGfxPrint's internal coordinate system.
			float worldOffsetX = ndcX * AEGfxGetWinMaxX();
			float worldOffsetY = ndcY * AEGfxGetWinMaxY();

			AEMtx33 s, t, finalMtx;
			AEMtx33Scale(&s, size, size);
			AEMtx33Trans(&t, camX + worldOffsetX, camY + worldOffsetY);
			AEMtx33Concat(&finalMtx, &t, &s);

			AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
			AEGfxSetBlendMode(AE_GFX_BM_BLEND);
			AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
			AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
			AEGfxTextureSet(tex, 0.0f, 0.0f);

			AEGfxSetTransform(finalMtx.m);
			AEGfxMeshDraw(g_pUnitSquare, AE_GFX_MDM_TRIANGLES);
			};

		// Draw the Tutorial Text and Sprites
		AEGfxSetBlendMode(AE_GFX_BM_BLEND); // Required for text transparency
		if (g_FontId >= 0) {
			AEGfxPrint(g_FontId, (char*)"HOW TO PLAY", -0.125f, 0.7f, 1.5f, 0.0f, 1.0f, 1.0f, 1.0f); // Cyan

			// ALIGNMENT GRID: Change these to shift the entire column left or right!
			float textX = -0.20f;
			float sprX = 0.05f;
			float yAdj = -0.60f; // Pushes the sprite up slightly to perfectly center with text

			// Basic Controls
			DrawUISprite(tut_EnokiTex, sprX, 0.4f + yAdj, 50.0f);
			AEGfxPrint(g_FontId, (char*)"WASD : Move Enoki Ninja", textX, 0.4f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

			// Item 1: Speed (Blue)
			DrawUISprite(tut_BlueMushTex, sprX, 0.2f + yAdj, 50.0f);
			AEGfxPrint(g_FontId, (char*)"1 : Blue Mushroom gives a Speed Boost!", textX, 0.2f, 1.0f, 0.2f, 0.6f, 1.0f, 1.0f);

			// Item 2: Stun (Green)
			DrawUISprite(tut_GreenMushTex, sprX, 0.0f + yAdj, 50.0f);
			AEGfxPrint(g_FontId, (char*)"2 : Green Mushroom Stuns all enemies!", textX, 0.0f, 1.0f, 0.2f, 1.0f, 0.2f, 1.0f);

			// Item 3: Vision (Red)
			DrawUISprite(tut_RedMushTex, sprX, -0.2f + yAdj, 50.0f);
			AEGfxPrint(g_FontId, (char*)"3 : Red Mushroom finds the nearest key!", textX, -0.2f, 1.0f, 1.0f, 0.2f, 0.2f, 1.0f);

			// Key & Door
			DrawUISprite(tut_KeyTex, sprX, -0.4f + yAdj, 50.0f);
			AEGfxPrint(g_FontId, (char*)"E : Unlock Exit Door (Requires 3 Keys)", textX, -0.4f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f);

			// Enemy Warning
			DrawUISprite(tut_EnemyTex, sprX, -0.6f + yAdj, 60.0f);
			AEGfxPrint(g_FontId, (char*)"Avoid the Pursuing Enemies!", textX, -0.6f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);

			// Start Prompt
			AEGfxPrint(g_FontId, (char*)"Press [ENTER] to Start", -0.15f, -0.85f, 1.2f, 0.0f, 1.0f, 0.0f, 1.0f); // Green
		}

		// Reset Engine State safely
		AEGfxTextureSet(nullptr, 0.0f, 0.0f);
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetBlendMode(AE_GFX_BM_NONE);
	}
	// -----------------------------
}

// Clears the logic and memory for the current level objects.
void Level_Free()
{
	// Iterates through rooms to clear neighbor pointers.
	for (auto& room : g_DungeonRooms) if (room) room->ClearNeighbours();
	// Clears the list of room pointers.
	g_DungeonRooms.clear();
	// Resets global pointers and flags.
	g_Character = nullptr;
	g_BossRoom = nullptr;
	g_ItemsInitialized = false;
}

// Releases all gpu assets and engine resources.
void Level_Unload()
{
	// Releases the floor textures from the gpu.
	TilesetManager::Unload();

	if (g_texWall) { AEGfxTextureUnload(g_texWall); g_texWall = nullptr; }

	if (g_FontId >= 0) {
		AEGfxDestroyFont(g_FontId);
		g_FontId = -1;
	}
	// --- UNLOAD TUTORIAL SPRITES ---
	if (tut_EnokiTex) { AEGfxTextureUnload(tut_EnokiTex); tut_EnokiTex = nullptr; }
	if (tut_EnemyTex) { AEGfxTextureUnload(tut_EnemyTex); tut_EnemyTex = nullptr; }
	if (tut_BlueMushTex) { AEGfxTextureUnload(tut_BlueMushTex); tut_BlueMushTex = nullptr; }
	if (tut_GreenMushTex) { AEGfxTextureUnload(tut_GreenMushTex); tut_GreenMushTex = nullptr; }
	if (tut_RedMushTex) { AEGfxTextureUnload(tut_RedMushTex); tut_RedMushTex = nullptr; }
	if (tut_KeyTex) { AEGfxTextureUnload(tut_KeyTex); tut_KeyTex = nullptr; }

	// Free the primary unit square mesh.
	if (g_pUnitSquare) {
		AEGfxMeshFree(g_pUnitSquare);
		g_pUnitSquare = nullptr;
	}

	// Free the rectangle outline mesh.
	if (g_pRectOutline) {
		AEGfxMeshFree(g_pRectOutline);
		g_pRectOutline = nullptr;
	}

	// Unload the inventory system assets.
	g_Inventory.Unload();

	// Unload settings menu resources.
	SettingsMenu_Unload();

	// Stop all active sound effects (heartbeat, footsteps, etc.)
	AudioSystem::StopGroup(false);

	// Release enemy textures and clear the list.
	for (auto& enemy : g_Enemies)
	{
		enemy.Unload();
	}
	// Jumpscare free
	if (g_pJumpscareTex) { AEGfxTextureUnload(g_pJumpscareTex); g_pJumpscareTex = nullptr; }
	// Empties the enemy container.
	g_Enemies.clear();
}