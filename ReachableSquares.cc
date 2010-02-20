#include "MoveDeciders.h"
#include <cstdio>
#include <cassert>

static int floodFill(std::vector<bool> &board, int x, int y,
        int width, int height)
{
    int ret = 1;

    board[x*height + y] = true;

    if (!board[(x - 1)*height + y])
        ret += floodFill(board, x - 1, y, width, height);

    if (!board[(x + 1)*height + y])
        ret += floodFill(board, x + 1, y, width, height);

    if (!board[x*height + (y - 1)])
        ret += floodFill(board, x, y - 1, width, height);

    if (!board[x*height + (y + 1)])
        ret += floodFill(board, x, y + 1, width, height);

    return ret;
}

int countReachableSquares(const Map &map, Player player)
{
    int width = map.width();
    int height = map.height();
    std::vector<bool> board(width*height);

    int x, y, oppX, oppY;
    switch (player) {
        case SELF:
            x = map.myX();
            y = map.myY();
            oppX = map.enemyX();
            oppY = map.enemyY();
            break;
        case ENEMY:
            x = map.enemyX();
            y = map.enemyY();
            oppX = map.myX();
            oppY = map.myY();
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
