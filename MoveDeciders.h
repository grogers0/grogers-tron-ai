#ifndef MOVE_DECIDERS_H
#define MOVE_DECIDERS_H

#include "Map.h"
#include <set>

bool isOpponentIsolated(const Map &map);
Direction decideMoveIsolatedFromOpponent(Map map);
int countReachableSquares(const Map &map, Player player);
Direction decideMoveMinimax(Map);
bool squaresReachEachOther(const Map &map, int x1, int x2, int y1, int y2);

#endif
