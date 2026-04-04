#ifndef TILESET_H
#define TILESET_H

#include "AEEngine.h"
#include <vector>

/**
 * @brief Enumeration for different tileset types.
 */
enum class TilesetType {
	Type_01,
	Type_02,
	Type_03,
	Type_04,
	Type_05,
	Count
};

/**
 * @brief Structure to hold data for a single tileset.
 */
struct TilesetData {
	float r, g, b;            /**< Rgb color values for the tileset. */
	AEGfxTexture* pTexture;   /**< Pointer to the loaded texture for this tileset. */
};

/**
 * @brief Manager class for handling dungeon tileset styles.
 */
class TilesetManager {
public:
	/**
	 * @brief Initializes and loads tileset data.
	 */
	static void Load();

	/**
	 * @brief Releases all loaded texture assets from the gpu memory.
	 */
	static void Unload();

	/**
	 * @brief Returns the data for a specific tileset type.
	 * @param type The tileset type to retrieve.
	 * @return A constant reference to the tileset data.
	 */
	static const TilesetData& Get(TilesetType type);

	/**
	 * @brief Returns a random tileset type from the available list.
	 * @return A randomly selected tileset type.
	 */
	static TilesetType GetRandom();

	static AEGfxTexture* s_pBossTexture;

private:
	static std::vector<TilesetData> s_List; 
};

#endif