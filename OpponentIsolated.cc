#include "MoveDeciders.h"
#include "Time.h"

#include <cstdio>
#include <stdexcept>
#include <climits>
#include <utility>
#include <cassert>


static bool floodFillReachesOtherSquare(std::vector<bool> &board,
        int x1, int y1, int x2, int y2)
{
    if (x1 == x2 && y1 == y2)
        return true;

    board[x1*height + y1] = true;

    if (!board[(x1 - 1)*height + y1] &&
            floodFillReachesOtherSquare(board, x1 - 1, y1, x2, y2))
        return true;

    if (!board[(x1 + 1)*height + y1] &&
            floodFillReachesOtherSquare(board, x1 + 1, y1, x2, y2))
        return true;

    if (!board[x1*height + (y1 - 1)] &&
            floodFillReachesOtherSquare(board, x1, y1 - 1, x2, y2))
        return true;

    if (!board[x1*height + (y1 + 1)] &&
            floodFillReachesOtherSquare(board, x1, y1 + 1, x2, y2))
        return true;

    return false;
}

bool squaresReachEachOther(const std::vector<bool> &boardOrig, int x1, int y1,
        int x2, int y2)
{
    std::vector<bool> board(boardOrig);

    board[x1*height + y1] = false;
    board[x2*height + y2] = false;

    return floodFillReachesOtherSquare(board, x1, y1, x2, y2);
}

bool isOpponentIsolated(const Map &map)
{
    return !squaresReachEachOther(map.getBoard(), map.myX(), map.myY(),
            map.enemyX(), map.enemyY());
}

static std::pair<int, int> isolatedPathFind(Map &map, int truedepth, int depth, Direction *outDir)
{
    if (Time::now() > deadline)
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
            fprintf(stderr, "isolated path depth %d ==> %s, found depth: %d, count: %d\n", depth, dirToString(dir), depthCount.first, depthCount.second);

            ++depth;
            depthCount = isolatedPathFind(map, 0, depth, &tmpDir);
        } while (depthCount.first >= depth);
    } catch (...) {
    }

    return dir;

}

