//author :Dyaln Lim
#ifndef ROOMTEMPLATE_H
#define ROOMTEMPLATE_H

#include <vector>

// bitwise flags for door directions
enum DoorDirection
{
    DOOR_NONE  = 0,
    DOOR_NORTH = 1,
    DOOR_SOUTH = 2,
    DOOR_EAST  = 4,
    DOOR_WEST  = 8,
    DOOR_ALL   = 15
};

// holds predefined grid and valid connection points
struct RoomTemplate
{
    // initialize variable to fix compiler warning
    int allowedDoors = 0;

    // two dimensional grid containing walls and floors
    std::vector<std::vector<int>> layout;
};

// manages available room designs
class TemplateManager
{
public:
    /**
     * @brief Initializes or clears the internal template memory list. Must be called before generation.
     */
    static void LoadTemplates();

    /**
     * @brief Retrieves a room layout design that satisfies the required door connections.
     * @param requiredDoors A bitmask (from DoorDirection) specifying necessary exits.
     * @return A compiled RoomTemplate structure containing the 2D tile layout.
     */
    static RoomTemplate GetRandomTemplate(int requiredDoors);

private:
    // stores all available room designs
    static std::vector<RoomTemplate> templates;
};

#endif