#include "MoveDeciders.h"
#include "Time.h"

#include <cstdio>
#include <stdexcept>
#include <climits>
#include <utility>
#include <cassert>


static bool floodFillReachesOtherSquare(std::vector<bool> &board,
        int x1, int y1, int x2, int y2, int height)
{
    if (x1 == x2 && y1 == y2)
        return true;

    board[x1*height + y1] = true;

    if (!board[(x1 - 1)*height + y1] &&
            floodFillReachesOtherSquare(board, x1 - 1, y1, x2, y2, height))
        return true;

    if (!board[(x1 + 1)*height + y1] &&
            floodFillReachesOtherSquare(board, x1 + 1, y1, x2, y2, height))
        return true;

    if (!board[x1*height + (y1 - 1)] &&
            floodFillReachesOtherSquare(board, x1, y1 - 1, x2, y2, height))
        return true;

    if (!board[x1*height + (y1 + 1)] &&
            floodFillReachesOtherSquare(board, x1, y1 + 1, x2, y2, height))
        return true;

    return false;
}

bool squaresReachEachOther(const std::vector<bool> &boardOrig, int x1, int y1,
        int x2, int y2, int height)
{
    std::vector<bool> board(boardOrig);

    board[x1*height + y1] = false;
    board[x2*height + y2] = false;

    return floodFillReachesOtherSquare(board, x1, y1, x2, y2, height);
}

bool isOpponentIsolated(const Map &map)
{
    return !squaresReachEachOther(map.getBoard(), map.myX(), map.myY(),
            map.enemyX(), map.enemyY(), map.height());
}

static std::pair<int, int> isolatedPathFind(Map &map, int depth, Direction *outDir)
{
    if (Time::now() > deadline)
        throw std::runtime_error("time expired");

    if (depth <= 0)
        return std::make_pair(depth, countReachableSquares(map, SELF));

    int bestCount = -INT_MAX;
    int bestDepth = INT_MAX;
    Direction bestDir = NORTH;

    for (Direction dir = DIR_MIN; dir <= DIR_MAX;
            dir = static_cast<Direction>(dir + 1)) {
        if (map.isWall(dir, SELF))
            continue;

        map.move(dir, SELF);
        std::pair<int, int> tmp = isolatedPathFind(map, depth - 1, NULL);
        map.unmove(dir, SELF);

        if (tmp.first < bestDepth) {
            bestDepth = tmp.first;
            bestCount = tmp.second;
            bestDir = dir;
        } else if (tmp.first == bestDepth &&
                tmp.second > bestCount) {
            bestCount = tmp.second;
            bestDir = dir;
        }
    }

    if (outDir)
        *outDir = bestDir;

    return std::make_pair(bestDepth, bestCount);
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
            depthCount = isolatedPathFind(map, depth, &tmpDir);
        } while (depthCount.first == 0);
    } catch (...) {
    }

    return dir;

}

