#include "Level.h"
#include "RoomGenerator.h"
#include "Player.h"
#include "GameStateManager.h"
#include "Level1.h"

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

    static Player g_Player;

// END ENTITY DATA

// Set this to true to see neighbors, false to see only the current room
static bool g_RevealNeighbors = true;

void Level_Load()
{
    // Replace 278 with the number from your latest leak report
    _CrtSetBreakAlloc(278);

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
}

void Level_Init()
{
    // Generator produces a centered dungeon layout based on our screen resolution
    RoomGenerator generator;

    // 4096 width, 4096 height, 256 room size
    g_DungeonRooms = generator.Generate(4096, 4096, 256);
    //g_DungeonRooms = generator.Generate(4096, 4096, 80, 600);

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
        g_Player.Init(startRoom->rect.GetCenter());
        startRoom->isDiscovered = true;

		// Only reveals neighbours if the toggle (check top of this file) is enabled
        if (g_RevealNeighbors)
        {
            for (auto* neighbor : startRoom->GetNeighbours()) {
                if (neighbor) neighbor->isDiscovered = true;
            }
        }
    }
}

void Level_Update()
{
    // If the R key is pressed, signal the system to rebuild the dungeon from scratch
    if (AEInputCheckTriggered(AEVK_R))
    {
        gGameStateNext = GS_RESTART;
    }

    float dt = (float)AEFrameRateControllerGetFrameTime();
    g_Player.Update(dt);

    // Centers the camera on the player to ensure they never walk off-screen
    AEGfxSetCamPosition(g_Player.pos.x, g_Player.pos.y);

    // Use a small epsilon to make the "Inside Room" check more forgiving
    float epsilon = 5.0f;

    // Iterates through the rooms to check if the player's position is within bounds
    // The Y-axis check confirms the player is between the bottom and top edges
    for (auto& room : g_DungeonRooms)
    {
        // Check if player's center is within the room boundaries
        if (g_Player.pos.x >= (room->rect.left - epsilon) &&
            g_Player.pos.x <= (room->rect.right + epsilon) &&
            g_Player.pos.y >= (room->rect.bottom - epsilon) &&
            g_Player.pos.y <= (room->rect.top + epsilon))
        {
            // Reveal the room the player is currently in
            room->isDiscovered = true;

            // Reveal all neighbors linked during FindNeighbours
            // Uses the pointer list stored inside the Room class
            // *** NOTE: Only reveal neighbors if the toggle (check top of this file) is enabled ***
            if (g_RevealNeighbors)
            {
                for (auto* neighbor : room->GetNeighbours())
                {
                    if (neighbor != nullptr)
                    {
                        neighbor->isDiscovered = true;
                    }
                }
            }

			// For debugging: Toggle neighbor reveal with the N key
            if (AEInputCheckTriggered(AEVK_N))
            {
                g_RevealNeighbors = !g_RevealNeighbors;
                printf("Neighbor Reveal: %s\n", g_RevealNeighbors ? "ON" : "OFF");
            }

            // Once the room the player is in is found, stop
            break;
        }
    }

    // Press Q to quit the game entirely
    if (AEInputCheckTriggered(AEVK_Q)) {
        gGameStateNext = GS_QUIT;
    }

    // Press ESC to go back to Main Menu
    if (AEInputCheckTriggered(AEVK_ESCAPE)) {
        gGameStateNext = GS_MAINMENU;
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
            AEGfxSetColorToMultiply(0.7f, 0.1f, 0.1f, 1.0f);
        else if (room->type == RoomType::Start)
            AEGfxSetColorToMultiply(0.1f, 0.5f, 0.1f, 1.0f);
        else
            AEGfxSetColorToMultiply(0.3f, 0.3f, 0.3f, 1.0f);

        // DRAW PASS 1: The solid floor using the Triangle mesh
        // Use the bias here to overlap slightly and hide triangle seams
        AEGfxMeshDraw(g_pUnitSquare, AE_GFX_MDM_TRIANGLES);

        // DRAW PASS 2: The clean outline
        // Switches the mesh to g_pRectOutline and use NO bias
        // Because g_pRectOutline has no internal vertices, the diagonal is gone
        AEMtx33Scale(&scale, (float)room->rect.width(), (float)room->rect.height());
        AEMtx33Concat(&transform, &trans, &scale);
        AEGfxSetTransform(transform.m);

        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f);
        AEGfxMeshDraw(g_pRectOutline, AE_GFX_MDM_LINES_STRIP);
    }

    // --- THE BOUNDARY LOGIC ---sa
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
    
    Level1_Free();
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
    // This is the mesh added to stop the diagonal lines displaying
    if (g_pRectOutline)
    {
        AEGfxMeshFree(g_pRectOutline);
        g_pRectOutline = nullptr;
    }

    // Release any textures or allocated data inside the player class
    g_Player.Unload();
}