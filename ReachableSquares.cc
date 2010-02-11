#include "MoveDeciders.h"
#include <cstdio>

size_t floodFill(std::vector<std::vector<bool> > &board, int x, int y,
        int width, int height)
{
    size_t ret = 1;

    board[x][y] = true;

    if (x > 0 && !board[x - 1][y])
        ret += floodFill(board, x - 1, y, width, height);

    if (x < width - 1 && !board[x + 1][y])
        ret += floodFill(board, x + 1, y, width, height);

    if (y > 0 && !board[x][y - 1])
        ret += floodFill(board, x, y - 1, width, height);

    if (y < height - 1 && !board[x][y + 1])
        ret += floodFill(board, x, y + 1, width, height);

    return ret;
}

size_t countReachableSquares(const Map &map, Player player)
{
    int width = map.width();
    int height = map.height();
    std::vector<std::vector<bool> > board(width, std::vector<bool>(height));

    int x, y;
    switch (player) {
        case SELF:
            x = map.myX();
            y = map.myY();
            break;
        case OPPONENT:
            x = map.opponentX();
            y = map.opponentY();
            break;
    }

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            board[i][j] = map.isWall(i, j);
        }
    }

    return floodFill(board, x, y, width, height);
}

std::set<Direction> getPossibleMovesReachableSquares(const Map &map)
{
   size_t bestReachable = 0;

    std::set<Direction> bestDirs;

    for (Direction dir = DIR_MIN; dir <= DIR_MAX;
            dir = static_cast<Direction>(dir + 1)) {
        if (!map.isWall(dir, SELF)) {
            Map newMap(map);
            newMap.move(dir, SELF);
            size_t reachable = countReachableSquares(newMap, SELF);
            fprintf(stderr, "Move %s has %u reachable squares\n", dirToString(dir), reachable);
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
