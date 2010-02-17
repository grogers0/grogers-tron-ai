#ifndef MOVE_DECIDERS_H
#define MOVE_DECIDERS_H

#include "Map.h"
#include <set>

bool isOpponentIsolated(const Map &map);
Direction decideMoveIsolatedFromOpponent(Map map);
int countReachableSquares(const Map &map, Player player);
int countReachableSmart(const Map &map, Player player);
std::set<Direction> getPossibleMovesReachableSquares(const Map &map);
Direction decideMoveMinimax(const Map &);

#endif
