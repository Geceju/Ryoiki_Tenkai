#pragma once

#ifndef LEVEL_H
#define LEVEL_H

// Setup phase for heavy assets like textures or permanent meshes
void Level_Load();

// Reset phase for level-specific variables and fresh dungeon generation
void Level_Init();

// Per-frame logic phase for input handling and movement
void Level_Update();

// Per-frame rendering phase for drawing rooms and UI
void Level_Draw();

// Cleanup phase for temporary level data when exiting
void Level_Free();

// Shutdown phase for clearing heavy assets from memory
void Level_Unload();

#endif