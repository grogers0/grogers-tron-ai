// MyTronBot.cc
//
// This is the code file that you will modify in order to create your entry.

#include "Map.h"
#include <string>
#include <vector>

Direction whichMove(const Map& map)
{
    int x = map.myX();
    int y = map.myY();

    Direction dir = NORTH;

    if (!map.isWall(x, y-1)) {
        return NORTH;
    }
    if (!map.isWall(x+1, y)) {
        return EAST;
    }
    if (!map.isWall(x, y+1)) {
        return SOUTH;
    }
    if (!map.isWall(x-1, y)) {
        return WEST;
    }
    return dir;
}

// Ignore this function. It is just handling boring stuff for you, like
// communicating with the Tron tournament engine.
int main()
{
    while (true)
    {
        Map map;
        Map::sendMoveToServer(whichMove(map));
    }
    return 0;
}
