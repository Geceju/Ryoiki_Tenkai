#include "Tilesets.h"
#include <random>
#include <iostream>

// Static list to store tileset data.
std::vector<TilesetData> TilesetManager::s_List;

// Initializes the tileset list with color and texture data.
void TilesetManager::Load() {
	// Clears the list to ensure no duplicate data on reload.
	s_List.clear();
	// Resizes the list to match the number of tileset types.
	s_List.resize((size_t)TilesetType::Count);

	// Type 01 initialization with gray color and a loaded texture.
	s_List[(int)TilesetType::Type_01] = { 0.5f, 0.5f, 0.5f, AEGfxTextureLoad((char*)"Assets/testimg.png") };
	// Type 02 initialization with dark blue color and a loaded texture.
	s_List[(int)TilesetType::Type_02] = { 0.2f, 0.2f, 0.4f, AEGfxTextureLoad((char*)"Assets/Tiles/Floor02.png") };
	// Type 03 initialization with dark green color and a loaded texture.
	s_List[(int)TilesetType::Type_03] = { 0.2f, 0.4f, 0.2f, AEGfxTextureLoad((char*)"Assets/Tiles/Floor03.png") };
	// Type 04 initialization with dark red color and a loaded texture.
	s_List[(int)TilesetType::Type_04] = { 0.4f, 0.2f, 0.2f, AEGfxTextureLoad((char*)"Assets/Tiles/Floor04.png") };
	// Type 05 initialization with purple color and a loaded texture.
	s_List[(int)TilesetType::Type_05] = { 0.4f, 0.2f, 0.4f, AEGfxTextureLoad((char*)"Assets/Tiles/Floor05.png") };

	// Iterates through the list to verify textures loaded correctly.
	for (size_t i = 0; i < s_List.size(); ++i) {
		// Checks if the texture pointer is null indicating a load failure.
		if (s_List[i].pTexture == nullptr) {
			// Logs a warning to the console identifying the failed tileset index.
			std::cout << "Warning: Tileset texture at index " << i << " failed to load. Falling back to color." << std::endl;
		}
	}
}

// Releases all loaded texture assets from the gpu memory.
void TilesetManager::Unload() {
	// Iterates through the list of tileset data.
	for (auto& style : s_List) {
		// Checks if the texture pointer is valid before attempting to unload.
		if (style.pTexture != nullptr) {
			// Calls the engine function to free the texture resource.
			AEGfxTextureUnload(style.pTexture);
			// Resets the pointer to null to prevent accidental access.
			style.pTexture = nullptr;
		}
	}
	// Clears the internal storage list.
	s_List.clear();
}

// Retrieves the tileset data for a specific type.
const TilesetData& TilesetManager::Get(TilesetType type) {
	// Returns the data at the specified index.
	return s_List[(int)type];
}

// Selects a random tileset type from the available options.
TilesetType TilesetManager::GetRandom() {
	// Static random number generator for selecting an index.
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, (int)TilesetType::Count - 1);

	// Returns the randomly generated tileset type.
	return (TilesetType)dis(gen);
}