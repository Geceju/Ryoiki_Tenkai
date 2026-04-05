//author :Dyaln Lim
#include "RoomTemplate.h"
#include "utils.h"

// initialize static list
std::vector<RoomTemplate> TemplateManager::templates;

void TemplateManager::LoadTemplates()
{
    // clear memory list
    templates.clear();
}

// helper function to stamp a two by two wall block
void PlaceBlock(std::vector<std::vector<int>>& layout, int x, int y)
{
    // prevent out of bounds
    if (x >= 1 && x <= 13 && y >= 1 && y <= 13)
    {
        layout[y][x] = 1;
        layout[y + 1][x] = 1;
        layout[y][x + 1] = 1;
        layout[y + 1][x + 1] = 1;
    }
}

// helper to stamp thick shape into quadrant
void StampThickShape(std::vector<std::vector<int>>& layout, int startX, int startY)
{
    // pick random structural design
    int shapeType = Random::Range(0, 4);

    if (shapeType == 0)
    {
        // thick letter l design
        PlaceBlock(layout, startX, startY);
        PlaceBlock(layout, startX, startY + 2);
        PlaceBlock(layout, startX, startY + 4);
        PlaceBlock(layout, startX + 2, startY + 4);
    }
    else if (shapeType == 1)
    {
        // thick letter t design
        PlaceBlock(layout, startX, startY);
        PlaceBlock(layout, startX + 2, startY);
        PlaceBlock(layout, startX + 4, startY);
        PlaceBlock(layout, startX + 2, startY + 2);
        PlaceBlock(layout, startX + 2, startY + 4);
    }
    else if (shapeType == 2)
    {
        // thick letter i design
        PlaceBlock(layout, startX, startY);
        PlaceBlock(layout, startX, startY + 2);
        PlaceBlock(layout, startX, startY + 4);
    }
    else if (shapeType == 3)
    {
        // thick minus symbol design
        PlaceBlock(layout, startX, startY);
        PlaceBlock(layout, startX + 2, startY);
        PlaceBlock(layout, startX + 4, startY);
    }
    else
    {
        // thick square design
        PlaceBlock(layout, startX, startY);
        PlaceBlock(layout, startX + 2, startY);
        PlaceBlock(layout, startX, startY + 2);
        PlaceBlock(layout, startX + 2, startY + 2);
    }
}

RoomTemplate TemplateManager::GetRandomTemplate(int requiredDoors)
{
    RoomTemplate result;

    // assign door requirements
    result.allowedDoors = requiredDoors;

    // populate base grid with floor tiles
    result.layout.resize(16, std::vector<int>(16, 0));

    // construct solid perimeter walls
    for (int i = 0; i < 16; ++i)
    {
        result.layout[0][i] = 1;
        result.layout[15][i] = 1;
        result.layout[i][0] = 1;
        result.layout[i][15] = 1;
    }

    // generate varied chunky obstacles anywhere inside the room
    int obstacleStyle = Random::Range(0, 1);

    if (obstacleStyle == 0)
    {
        // scatter random two by two noise blocks
        for (int y = 2; y < 13; y += 2)
        {
            for (int x = 2; x < 13; x += 2)
            {
                if (Random::Range(0, 100) < 30)
                {
                    PlaceBlock(result.layout, x, y);
                }
            }
        }
    }
    else
    {
        // stamp thick shapes into safe corner quadrants
        // top left quadrant
        if (Random::Range(0, 100) > 30)
        {
            StampThickShape(result.layout, 2, 2);
        }

        // top right quadrant
        if (Random::Range(0, 100) > 30)
        {
            StampThickShape(result.layout, 8, 2);
        }

        // bottom left quadrant
        if (Random::Range(0, 100) > 30)
        {
            StampThickShape(result.layout, 2, 8);
        }

        // bottom right quadrant
        if (Random::Range(0, 100) > 30)
        {
            StampThickShape(result.layout, 8, 8);
        }
    }

    // guarantee exit paths by carving a wide safe cross from doors to center
    // clear center area completely
    for (int y = 6; y <= 9; ++y)
    {
        for (int x = 6; x <= 9; ++x)
        {
            result.layout[y][x] = 0;
        }
    }

    // carve top entry safe path
    if (requiredDoors & DOOR_NORTH)
    {
        for (int y = 0; y <= 7; ++y)
        {
            result.layout[y][7] = 0;
            result.layout[y][8] = 0;
        }
    }

    // carve bottom entry safe path
    if (requiredDoors & DOOR_SOUTH)
    {
        for (int y = 8; y <= 15; ++y)
        {
            result.layout[y][7] = 0;
            result.layout[y][8] = 0;
        }
    }

    // carve left entry safe path
    if (requiredDoors & DOOR_WEST)
    {
        for (int x = 0; x <= 7; ++x)
        {
            result.layout[7][x] = 0;
            result.layout[8][x] = 0;
        }
    }

    // carve right entry safe path
    if (requiredDoors & DOOR_EAST)
    {
        for (int x = 8; x <= 15; ++x)
        {
            result.layout[7][x] = 0;
            result.layout[8][x] = 0;
        }
    }

    return result;
}