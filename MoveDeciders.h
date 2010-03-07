#ifndef MOVE_DECIDERS_H
#define MOVE_DECIDERS_H

#include "Map.h"
#include <set>

bool isOpponentIsolated(const Map &map);
Direction decideMoveIsolatedFromOpponent(Map map);
int countReachableSquares(const Map &map, Player player);
Direction decideMoveMinimax(Map);
bool squaresReachEachOther(const std::vector<bool> &board,
        position pos1, position pos2);
void fillUnreachableSquares(std::vector<bool> &board, position pos);
bool isCorridorSquare(const std::vector<bool> &board, position pos);

#endif
