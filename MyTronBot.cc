// MyTronBot.cc
//
// This is the code file that you will modify in order to create your entry.

#include "Map.h"
#include "MoveDeciders.h"
#include "Time.h"
#include <vector>
#include <cstdio>
#include <set>
#include <iterator>
#include <algorithm>


Direction whichMove(const Map& map)
{
    if (isOpponentIsolated(map))
        return decideMoveIsolatedFromOpponent(map);

    return decideMoveMinimax(map);
}

// Ignore this function. It is just handling boring stuff for you, like
// communicating with the Tron tournament engine.
int main()
{
    Map map;

    while (true)
    {
        map.readFromFile(stdin);

        Time tincr(0, 800); // destroying the game tree takes a while...
        deadline = Time::now() + tincr;

        Direction dir = whichMove(map);
        Map::sendMoveToServer(dir);
    }
    return 0;
}
