#include "MoveDeciders.h"
#include "Time.h"

#include <cstdio>
#include <stdexcept>
#include <climits>
#include <utility>
#include <cassert>


static bool floodFillReachesOpponent(std::vector<bool> &board,
        int x, int y, int width, int height, int oppX, int oppY)
{
    if (x == oppX && y == oppY)
        return true;

    board[x*height + y] = true;

    if (!board[(x - 1)*height + y] &&
            floodFillReachesOpponent(board, x - 1, y, width, height, oppX, oppY))
        return true;

    if (!board[(x + 1)*height + y] &&
            floodFillReachesOpponent(board, x + 1, y, width, height, oppX, oppY))
        return true;

    if (!board[x*height + (y - 1)] &&
            floodFillReachesOpponent(board, x, y - 1, width, height, oppX, oppY))
        return true;

    if (!board[x*height + (y + 1)] &&
            floodFillReachesOpponent(board, x, y + 1, width, height, oppX, oppY))
            return true;

    return false;
}


bool isOpponentIsolated(const Map &map)
{
    int width = map.width();
    int height = map.height();

    std::vector<bool> board(width*height);
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            board[i*height + j] = map.isWall(i, j);
        }
    }

    board[map.myX()*height + map.myY()] = false;
    board[map.enemyX()*height + map.enemyY()] = false;

    return !floodFillReachesOpponent(board, map.myX(), map.myY(),
                width, height, map.enemyX(), map.enemyY());
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

