#include "Level.h"
#include "RoomGenerator.h"
#include "Player.h"
#include "GameStateManager.h"

// We maintain these pointers globally so they persist across the frame-based 
// lifecycle of the level from initialization through to the final draw call.
static std::vector<std::unique_ptr<Room>> g_DungeonRooms;
static AEGfxVertexList* g_pUnitSquare = nullptr;
static Player g_Player;

void Level_Load()
{
    // We define a standard square mesh that serves as the template for every room floor.
    // By using a unit size of 1.0, we can easily scale this to any room size later.
    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
        0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
        0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    g_pUnitSquare = AEGfxMeshEnd();
}

void Level_Init()
{
    // The generator produces a centered dungeon layout based on our screen resolution.
    RoomGenerator generator;

    // 4096 width, 4096 height, 256 room size
    g_DungeonRooms = generator.Generate(4096, 4096, 256);
    //g_DungeonRooms = generator.Generate(4096, 4096, 80, 600);

    // We start the player at the center of the first room and reveal it immediately.
    // Instead of spawning at index 0, we search for the room the generator specifically tagged as the 'Start' room based on center-proximity.
    Room* startRoom = nullptr;

    for (const auto& room : g_DungeonRooms)
    {
        if (room->type == RoomType::Start)
        {
            startRoom = room.get();
            break;
        }
    }

    // If a start room was found, we place the player at its center and reveal it.
    if (startRoom)
    {
        g_Player.Init(startRoom->rect.GetCenter());
        startRoom->isDiscovered = true;
    }
}

void Level_Update()
{
    // If the R key is pressed, we signal the system to rebuild the dungeon from scratch.
    if (AEInputCheckTriggered(AEVK_R))
    {
        gGameStateNext = GS_RESTART;
    }

    float dt = (float)AEFrameRateControllerGetFrameTime();
    g_Player.Update(dt);

    // We center the camera on the player to ensure they never walk off-screen.
    AEGfxSetCamPosition(g_Player.pos.x, g_Player.pos.y);

    // We iterate through the rooms to check if the player's position is within bounds.
    // The Y-axis check confirms the player is between the bottom and top edges.
    for (auto& room : g_DungeonRooms)
    {
        if (g_Player.pos.x >= room->rect.left && g_Player.pos.x <= room->rect.right &&
            g_Player.pos.y >= room->rect.bottom && g_Player.pos.y <= room->rect.top)
        {
            // Marking the current room as discovered clears the Fog of War.
            room->isDiscovered = true;

            // We also reveal neighboring rooms to show the player potential doorways and walls.
            for (auto* neighbor : room->GetNeighbours())
            {
                neighbor->isDiscovered = true;
            }
        }
    }
}

void Level_Draw()
{
    // We set the background to black to emphasize the discovered dungeon rooms.
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    // We iterate through all generated rooms to render their floors and walls.
    for (const auto& room : g_DungeonRooms)
    {
        // We skip rooms that have not yet been discovered by the player.
        if (!room->isDiscovered)
        {
            continue;
        }

        AEVec2 center = room->rect.GetCenter();
        AEMtx33 scale, trans, transform;

        // We scale the unit square to match the specific dimensions of each room.
        AEMtx33Scale(&scale, (float)room->rect.width(), (float)room->rect.height());
        AEMtx33Trans(&trans, center.x, center.y);
        AEMtx33Concat(&transform, &trans, &scale);
        AEGfxSetTransform(transform.m);

        // We apply the room tint based on its gameplay type.
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

        // Draw the floor and then the wall outline.
        AEGfxMeshDraw(g_pUnitSquare, AE_GFX_MDM_TRIANGLES);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f);
        AEGfxMeshDraw(g_pUnitSquare, AE_GFX_MDM_LINES_STRIP);
    }

    // THE BOUNDARY LOGIC IS OUTSIDE THE LOOP:
    // We draw the world border here so it appears on top of the rooms but only draws once.
    AEMtx33 bScale, bTrans, bTransform;

    // We scale the unit square to the world size we want.
    AEMtx33Scale(&bScale, 4096.0f, 4096.0f);
    AEMtx33Trans(&bTrans, 0.0f, 0.0f);
    AEMtx33Concat(&bTransform, &bTrans, &bScale);

    AEGfxSetTransform(bTransform.m);

    // We use a semi-transparent white color for the boundary line.
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 0.5f);
    AEGfxMeshDraw(g_pUnitSquare, AE_GFX_MDM_LINES_STRIP);

    // Finally, we draw the player on top of everything.
    g_Player.Draw();
}

void Level_Free()
{
    // We clear the list of rooms to free up memory before the next state transition.
    g_DungeonRooms.clear();
}

void Level_Unload()
{
    // We release all GPU assets to ensure there are no lingering memory leaks.
    if (g_pUnitSquare)
    {
        AEGfxMeshFree(g_pUnitSquare);
        g_pUnitSquare = nullptr;
    }
    g_Player.Unload();
}