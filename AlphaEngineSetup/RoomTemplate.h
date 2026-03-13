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
    // load hardcoded rooms into memory
    static void LoadTemplates();

    // fetch layout matching required doors
    static RoomTemplate GetRandomTemplate(int requiredDoors);

private:
    // stores all available room designs
    static std::vector<RoomTemplate> templates;
};

#endif