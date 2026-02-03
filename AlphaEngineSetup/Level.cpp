#include "Level.h"
#include "RoomGenerator.h"
#include "jogo.h"
#include "GameStateManager.h"
#include "Items.h"


// items manager static variables
static ItemsManager g_ItemsManager;
static bool g_ItemsInitialized = false;

/** * DUNGEON DATA
 * Stores the rooms using unique_ptr to ensure that memory is automatically
 * reclaimed when the vector is cleared or the application closes
 */
static std::vector<std::unique_ptr<Room>> g_DungeonRooms;

// GEOMETRY DATA (The "Data")
// These pointers represent the raw vertex information stored on the GPU

	// g_pUnitSquare: The "Fill" data
	// Uses triangles to create a solid surface for colors and future tileset textures
static AEGfxVertexList* g_pUnitSquare = nullptr;

// g_pRectOutline: The "Display" border
// A clean, 4-sided loop that excludes internal triangle edges to prevent diagonal lines
static AEGfxVertexList* g_pRectOutline = nullptr;

// END GEOMETRY DATA

// ENTITY DATA
// The player object maintains its own state, including position and discovery logic
// Initializing with a tile size of 256 to match the dungeon generation

static Character g_Player(0, 0, 256.0f);

// END ENTITY DATA

// Set this to true to see neighbors, false to see only the current room
static bool g_RevealNeighbors = true;

void Level_Load()
{
	// THE SEAMLESS FLOOR MESH (Triangles)
	// - Used for individual rooms
	// - Proper UV mapping (0.0 to 1.0) prevents the GPU from showing the internal triangle split.
	AEGfxMeshStart();

	AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

	g_pUnitSquare = AEGfxMeshEnd();

	// THE CLEAN OUTLINE MESH (Line Strip)
	// - Used for both the room borders AND the world boundary
	// - By only using 5 vertices in a loop, we physically remove the diagonal.
	AEGfxMeshStart();

	AEGfxVertexAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);   // BL
	AEGfxVertexAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f);    // BR
	AEGfxVertexAdd(0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f);     // TR
	AEGfxVertexAdd(-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);    // TL
	AEGfxVertexAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);   // Close loop

	g_pRectOutline = AEGfxMeshEnd();

	// Initialize the player assets
	g_Player.Load();

	// Initialize items graphics (AFTER engine is ready)
	g_ItemsManager.InitializeGraphics();
}

void Level_Init()
{
	// Generator produces a centered dungeon layout based on our screen resolution
	RoomGenerator generator;

	// 4096 width, 4096 height, 256 room size
	g_DungeonRooms = generator.Generate(4096, 4096, 256);

	// Start the player at the center of the first room and reveal it immediately
	// Instead of spawning at index 0, search for the room the generator specifically tagged as the 'Start' room based on center-proximity
	Room* startRoom = nullptr;

	for (const auto& room : g_DungeonRooms)
	{
		if (room->type == RoomType::Start)
		{
			startRoom = room.get();
			break;
		}
	}

	// If a start room was found, place the player at its center and reveal it
	if (startRoom)
	{
		// Get the world-space center of the starting room
		AEVec2 center = startRoom->rect.GetCenter();

		// Convert world-space coordinates to absolute grid units
		// If the room center is at (-512, -512) and tileSize is 256, 
		// the grid position should be (-2, -2).
		int gX = static_cast<int>(floor(center.x / 256.0f));
		int gY = static_cast<int>(floor(center.y / 256.0f));

		g_Player.SetPosition(gX, gY);
		startRoom->isDiscovered = true;

		// Only reveals neighbours if the toggle (check top of this file) is enabled
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
					float offsetX = (std::rand() % 100 - 50) * 2.0f; // ±100 pixels
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

void Level_Update()
{
	if (AEInputCheckTriggered(AEVK_R))
	{
		gGameStateNext = GS_RESTART;
	}

	// Pass the actual dungeon rooms for proper collision detection
	g_Player.Update(g_DungeonRooms);

	// Sync camera and discovery logic
	// Added half the tile size (128.0f) to match the centered player drawing logic
	float worldPosX = (float)g_Player.GetGridX() * 256.0f + 128.0f;
	float worldPosY = (float)g_Player.GetGridY() * 256.0f + 128.0f;

	// Set camera position to the center of the current tile
	AEGfxSetCamPosition(worldPosX, worldPosY);

	// Discovery logic
	float epsilon = 5.0f;
	for (auto& room : g_DungeonRooms)
	{
		if (worldPosX >= (room->rect.left - epsilon) &&
			worldPosX <= (room->rect.right + epsilon) &&
			worldPosY >= (room->rect.bottom - epsilon) &&
			worldPosY <= (room->rect.top + epsilon))
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

	// For debugging: Toggle neighbor reveal with the N key
	if (AEInputCheckTriggered(AEVK_N))
	{
		g_RevealNeighbors = !g_RevealNeighbors;
		printf("Neighbor Reveal: %s\n", g_RevealNeighbors ? "ON" : "OFF");
	}

	// Press Q to quit the game entirely
	if (AEInputCheckTriggered(AEVK_Q))
	{
		gGameStateNext = GS_QUIT;
	}

	// Update items with player's WORLD position
	float playerWorldX = static_cast<float>(g_Player.GetGridX()) * 256.0f + 128.0f;
	float playerWorldY = static_cast<float>(g_Player.GetGridY()) * 256.0f + 128.0f;

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

void Level_Draw()
{
	// We set the background to black to emphasize the discovered dungeon rooms
	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);

	// We iterate through all generated rooms to render their floors and walls
	for (const auto& room : g_DungeonRooms)
	{
		// If room not discovered, do not draw
		if (!room->isDiscovered)
		{
			continue;
		}

		AEVec2 center = room->rect.GetCenter();
		AEMtx33 scale, trans, transform;

		// Use a tiny bias (1.01) for the floor triangles. 
		// This causes rooms to overlap slightly, which prevents the background 
		// from bleeding through the diagonal line where the two triangles meet
		float bias = 1.01f;
		AEMtx33Scale(&scale, (float)room->rect.width() * bias, (float)room->rect.height() * bias);
		AEMtx33Trans(&trans, center.x, center.y);
		AEMtx33Concat(&transform, &trans, &scale);
		AEGfxSetTransform(transform.m);

		// Applies the room tint based on its gameplay type
		if (room->type == RoomType::Boss)
		{
			AEGfxSetColorToMultiply(0.7f, 0.1f, 0.1f, 1.0f);
		}
		else if (room->type == RoomType::Start)
		{
			AEGfxSetColorToMultiply(0.1f, 0.5f, 0.1f, 1.0f);
		}
		else
		{
			AEGfxSetColorToMultiply(0.3f, 0.3f, 0.3f, 1.0f);
		}

		// DRAW PASS 1: The solid floor using the Triangle mesh
		AEGfxMeshDraw(g_pUnitSquare, AE_GFX_MDM_TRIANGLES);

		// DRAW PASS 2: The clean outline
		// Switches the mesh to g_pRectOutline and use NO bias
		AEMtx33Scale(&scale, (float)room->rect.width(), (float)room->rect.height());
		AEMtx33Concat(&transform, &trans, &scale);
		AEGfxSetTransform(transform.m);

		AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f);
		AEGfxMeshDraw(g_pRectOutline, AE_GFX_MDM_LINES_STRIP);
	}

	// --- THE BOUNDARY LOGIC ---
	// Applies the same logic here where the outline mesh is used so the 4096x4096 
	// world border doesn't have a giant white line cutting through the middle
	AEMtx33 bScale, bTrans, bTransform;
	AEMtx33Scale(&bScale, 4096.0f, 4096.0f);
	AEMtx33Trans(&bTrans, 0.0f, 0.0f);
	AEMtx33Concat(&bTransform, &bTrans, &bScale);

	AEGfxSetTransform(bTransform.m);

	// Uses a semi-transparent white color for the boundary line
	AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 0.5f);

	// Draw the clean outline mesh instead of the triangle mesh
	AEGfxMeshDraw(g_pRectOutline, AE_GFX_MDM_LINES_STRIP);

	// --- DRAW ITEMS ONLY IN DISCOVERED ROOMS ---
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
				AEMtx33Scale(&scale, 48.0f * item.radius * 2.0f, 48.0f * item.radius * 2.0f);
				AEMtx33Trans(&trans, item.x, item.y);
				AEMtx33Concat(&transform, &trans, &scale);

				AEGfxSetColorToMultiply(item.color[0], item.color[1],
					item.color[2], item.color[3]);
				AEGfxSetTransform(transform.m);
				AEGfxMeshDraw(itemMesh, AE_GFX_MDM_TRIANGLES);
			}
		}
	}

	// Draws the player on top of everything
	g_Player.Draw();
}

void Level_Free()
{
	// Clear neighbor pointers first to break circular dependencies and allow deletion
	for (auto& room : g_DungeonRooms)
	{
		if (room)
		{
			room->ClearNeighbours();
		}
	}

	// Delete all Room objects now that connections are severed
	g_DungeonRooms.clear();

	// Release the static vector's internal capacity to the OS immediately
	g_DungeonRooms.shrink_to_fit();

	// Reset items
	g_ItemsInitialized = false;
}

void Level_Unload()
{
	// Releases the 'Fill' data (triangles) from the GPU
	if (g_pUnitSquare)
	{
		AEGfxMeshFree(g_pUnitSquare);
		g_pUnitSquare = nullptr;
	}

	// Releases the 'Outline' data (loop) from the GPU
	if (g_pRectOutline)
	{
		AEGfxMeshFree(g_pRectOutline);
		g_pRectOutline = nullptr;
	}

	// Release any textures or allocated data inside the player class
	g_Player.Unload();
}