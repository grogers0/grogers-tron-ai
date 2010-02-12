// MyTronBot.cc
//
// This is the code file that you will modify in order to create your entry.

#include "Map.h"
#include "MoveDeciders.h"
#include <vector>
#include <cstdio>
#include <set>
#include <iterator>
#include <algorithm>


Direction whichMove(const Map& map)
{
    return decideMoveIsolatedFromOpponent(map);
}

// Ignore this function. It is just handling boring stuff for you, like
// communicating with the Tron tournament engine.
int main(int argc, char **argv)
{
    Map map;

    if (argc < 2)
        return 0;

    char *moves = &argv[1][0];

    while (true)
    {
        map.readFromFile(stdin);

        Direction dir;

        if (*moves != '\0') {
            switch (tolower(*moves)) {
                case 'n': dir = NORTH; break;
                case 'e': dir = EAST; break;
                case 'w': dir = WEST; break;
                default:
                case 's': dir = SOUTH; break;
            }
            ++moves;
        } else {
            dir = whichMove(map);
        }

        //fprintf(stderr, "Moving %s\n", dirToString(dir));
        Map::sendMoveToServer(dir);
    }
    return 0;
}
