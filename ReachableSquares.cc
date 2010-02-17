#include "MoveDeciders.h"
#include <cstdio>

static int floodFill(std::vector<bool> &board, int x, int y,
        int width, int height)
{
    int ret = 1;

    board[x*height + y] = true;

    if (x > 0 && !board[(x - 1)*height + y])
        ret += floodFill(board, x - 1, y, width, height);

    if (x < width - 1 && !board[(x + 1)*height + y])
        ret += floodFill(board, x + 1, y, width, height);

    if (y > 0 && !board[x*height + (y - 1)])
        ret += floodFill(board, x, y - 1, width, height);

    if (y < height - 1 && !board[x*height + (y + 1)])
        ret += floodFill(board, x, y + 1, width, height);

    return ret;
}

int countReachableSquares(const Map &map, Player player)
{
    int width = map.width();
    int height = map.height();
    std::vector<bool> board(width*height);

    int x, y;
    switch (player) {
        case SELF:
            x = map.myX();
            y = map.myY();
            break;
        case ENEMY:
            x = map.enemyX();
            y = map.enemyY();
            break;
    }

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            board[i*height + j] = map.isWall(i, j);
        }
    }

    board[map.myX()*height + map.myY()] = true;
    board[map.enemyX()*height + map.enemyY()] = true;

    return floodFill(board, x, y, width, height) - 1;
}

std::set<Direction> getPossibleMovesReachableSquares(const Map &map)
{
    int bestReachable = 0;

    std::set<Direction> bestDirs;

    for (Direction dir = DIR_MIN; dir <= DIR_MAX;
            dir = static_cast<Direction>(dir + 1)) {
        if (!map.isWall(dir, SELF)) {
            Map newMap(map);
            newMap.move(dir, SELF);
            int reachable = countReachableSquares(newMap, SELF);
            if (reachable > bestReachable) {
                bestDirs.clear();
                bestDirs.insert(dir);
                bestReachable = reachable;
            } else if (reachable == bestReachable) {
                bestDirs.insert(dir);
            }
        }
    }

    return bestDirs;
}
