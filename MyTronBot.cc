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
    static bool firstTime = true;
    if (firstTime) {
        firstTime = false;
        Time tincr(2, 800);
        deadline = Time::now() + tincr;
    } else {
        Time tincr(0, 950);
        deadline = Time::now() + tincr;
    }

    if (isOpponentIsolated(map)) {
        return decideMoveIsolatedFromOpponent(map);
    }

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

        Direction dir = whichMove(map);

        Map::sendMoveToServer(dir);
    }
    return 0;
}
