#include "Level.h"
#include "RoomGenerator.h"
#include "jogo.h"
#include "GameStateManager.h"
#include "Tilesets.h" // [NEW] Include this to access the TilesetManager

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

    // Press ESC to go back to Main Menu
    if (AEInputCheckTriggered(AEVK_ESCAPE))
    {
        gGameStateNext = GS_MAINMENU;
    }
}

void Level_Draw()
{
    // Set the background to black so the dungeon rooms stand out
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    // Iterate through all generated rooms
    for (const auto& room : g_DungeonRooms)
    {
        // Optimization: Do not draw rooms the player hasn't found yet
        if (!room->isDiscovered)
        {
            continue;
        }

        // [1] Retrieve the specific color data for this room's style
        const TilesetData& style = TilesetManager::Get(room->tilesetID);

        // Calculate Position and Scale
        AEVec2 center = room->rect.GetCenter();
        AEMtx33 scale, trans, transform;

        // --- DRAW PASS 1: THE FLOOR (Colored Tile) ---

        // Use a tiny bias (1.01f) to make rooms overlap slightly. 
        // This prevents hairline cracks/lines from appearing between connected rooms.
        float bias = 1.01f;
        AEMtx33Scale(&scale, (float)room->rect.width() * bias, (float)room->rect.height() * bias);
        AEMtx33Trans(&trans, center.x, center.y);
        AEMtx33Concat(&transform, &trans, &scale);
        AEGfxSetTransform(transform.m);

        // COLOR LOGIC: 
        // 1. Boss Room = Always RED
        // 2. Start Room = Always GREEN
        // 3. Normal Room = Use the assigned Random Theme Color
        if (room->type == RoomType::Boss)
        {
            AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f); // Bright Red
        }
        else if (room->type == RoomType::Start)
        {
            AEGfxSetColorToMultiply(0.0f, 1.0f, 0.0f, 1.0f); // Bright Green
        }
        else
        {
            // Apply the color defined in Tilesets.cpp
            AEGfxSetColorToMultiply(style.r, style.g, style.b, 1.0f);
        }

        // Draw the solid square mesh
        AEGfxMeshDraw(g_pUnitSquare, AE_GFX_MDM_TRIANGLES);


        // --- THE WALLS (Black Outline) ---

        // Reset scale to exact size (no bias) for the outline
        AEMtx33Scale(&scale, (float)room->rect.width(), (float)room->rect.height());
        AEMtx33Concat(&transform, &trans, &scale);
        AEGfxSetTransform(transform.m);

        // Draw the outline in Black to visually separate the rooms
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f);
        AEGfxMeshDraw(g_pRectOutline, AE_GFX_MDM_LINES_STRIP);
    }

    // --- THE WORLD BOUNDARY ---

    // Draw the 4096x4096 boundary so the player knows the limits of the world
    AEMtx33 bScale, bTrans, bTransform;
    AEMtx33Scale(&bScale, 4096.0f, 4096.0f);
    AEMtx33Trans(&bTrans, 0.0f, 0.0f);
    AEMtx33Concat(&bTransform, &bTrans, &bScale);

    AEGfxSetTransform(bTransform.m);

    // Draw boundary as semi-transparent white
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 0.5f);
    AEGfxMeshDraw(g_pRectOutline, AE_GFX_MDM_LINES_STRIP);

    // --- THE PLAYER ---
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