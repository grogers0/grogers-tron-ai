#include "MoveDeciders.h"

#include <cstdio>
#include <stdexcept>
#include <climits>
#include <utility>
#include <cassert>


static bool floodFillReachesOtherSquare(std::vector<bool> &board,
        position pos1, position pos2)
{
    if (pos1 == pos2)
        return true;

    board[index(pos1)] = true;

    if (!board[index(pos1.north())] &&
            floodFillReachesOtherSquare(board, pos1.north(), pos2))
        return true;

    if (!board[index(pos1.south())] &&
            floodFillReachesOtherSquare(board, pos1.south(), pos2))
        return true;

    if (!board[index(pos1.west())] &&
            floodFillReachesOtherSquare(board, pos1.west(), pos2))
        return true;

    if (!board[index(pos1.east())] &&
            floodFillReachesOtherSquare(board, pos1.east(), pos2))
        return true;

    return false;
}

bool squaresReachEachOther(const std::vector<bool> &boardOrig,
        position pos1, position pos2)
{
    std::vector<bool> board(boardOrig);

    board[index(pos1)] = false;
    board[index(pos2)] = false;

    return floodFillReachesOtherSquare(board, pos1, pos2);
}

bool isOpponentIsolated(const Map &map)
{
    return !squaresReachEachOther(map.getBoard(), map.my_pos(), map.enemy_pos());
}

static std::pair<int, int> isolatedPathFind(Map &map, int truedepth, int depth, Direction *outDir)
{
    if (time_expired)
        throw std::runtime_error("time expired");

    if (depth <= 0)
        return std::make_pair(truedepth, countReachableSquares(map, SELF));

    int bestCount = 0;
    int bestTrueDepth = truedepth;
    Direction bestDir = NORTH;

    // singularity enhancement
    int newdepth = depth - 1;
    if (map.cntMoves(SELF) == 1)
        newdepth = depth;

    for (Direction dir = DIR_MIN; dir <= DIR_MAX;
            dir = static_cast<Direction>(dir + 1)) {
        if (map.isWall(dir, SELF))
            continue;

        map.move(dir, SELF);
        std::pair<int, int> tmp = isolatedPathFind(map, truedepth + 1, newdepth, NULL);
        map.unmove(dir, SELF);

        if (tmp.first + tmp.second > bestTrueDepth + bestCount) {
            bestTrueDepth = tmp.first;
            bestCount = tmp.second;
            bestDir = dir;
        }
    }

    if (outDir)
        *outDir = bestDir;

    return std::make_pair(bestTrueDepth, bestCount);
}

Direction decideMoveIsolatedFromOpponent(Map map)
{
    Direction dir = NORTH, tmpDir = NORTH;
    std::pair<int, int> depthCount;

    try {
        int depth = 0;
        do {
            dir = tmpDir;
            //fprintf(stderr, "isolated path depth %d ==> %s, found depth: %d, count: %d\n", depth, dirToString(dir), depthCount.first, depthCount.second);

            ++depth;
            depthCount = isolatedPathFind(map, 0, depth, &tmpDir);
        } while (depthCount.first >= depth);
    } catch (...) {
    }

    return dir;

}

