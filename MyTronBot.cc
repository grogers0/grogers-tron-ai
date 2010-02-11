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


std::set<Direction> moveTowardsOpponent(const Map &map)
{
    // don't care what direction the opponent is in if we can't reach them
    if (isOpponentIsolated(map)) {
        fprintf(stderr, "Opponent is isolated, do not move towards them\n");
        return std::set<Direction>();
    }

    fprintf(stderr, "Opponent is reachable, try to move towards them\n");

    int diffX = map.myX() - map.opponentX();
    int diffY = map.myY() - map.opponentY();
    std::set<Direction> ret;

    // only try to move towards them if they aren't right on our ass
    if (abs(diffX) > 1 || abs(diffY) > 1) {
        if (diffX > 0)
            ret.insert(WEST);
        else if (diffX < 0)
            ret.insert(EAST);

        if (diffY > 0)
            ret.insert(NORTH);
        else if (diffY < 0)
            ret.insert(SOUTH);
    }

    return ret;
}

Direction whichMove(const Map& map)
{
    if (isOpponentIsolated(map))
        return decideMoveIsolatedFromOpponent(map);

    int x = map.myX();
    int y = map.myY();

    std::set<Direction> moves1 = getPossibleMovesReachableSquares(map);
    std::set<Direction> moves2 = moveTowardsOpponent(map);

    fprintf(stderr, "Possible reachable square moves:");
    for (std::set<Direction>::const_iterator it = moves1.begin(); it != moves1.end(); ++it)
        fprintf(stderr, " %s", dirToString(*it));
    fprintf(stderr, "\nPossible towards opponent moves:");
    for (std::set<Direction>::const_iterator it = moves2.begin(); it != moves2.end(); ++it)
        fprintf(stderr, " %s", dirToString(*it));
    fprintf(stderr, "\n");


    std::vector<Direction> intersection;
    std::set_intersection(moves1.begin(), moves1.end(),
            moves2.begin(), moves2.end(), std::back_inserter(intersection));
    if (intersection.empty())
        return *moves1.begin();

    fprintf(stderr, "Possible remainder moves:");
    for (std::vector<Direction>::const_iterator it = intersection.begin(); it != intersection.end(); ++it)
        fprintf(stderr, " %s", dirToString(*it));
    fprintf(stderr, "\n");

    return *intersection.begin();
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
        fprintf(stderr, "Moving %s\n", dirToString(dir));
        Map::sendMoveToServer(dir);
    }
    return 0;
}
