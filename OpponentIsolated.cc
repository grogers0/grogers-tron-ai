#include "MoveDeciders.h"



bool floodFillReachesOpponent(std::vector<std::vector<bool> > &board,
        int x, int y, int width, int height, int oppX, int oppY)
{
    if (x == oppX && y == oppY)
        return true;

    board[x][y] = true;

    if (x > 0 && !board[x - 1][y] &&
            floodFillReachesOpponent(board, x - 1, y, width, height, oppX, oppY))
        return true;

    if (x < width - 1 && !board[x + 1][y] &&
            floodFillReachesOpponent(board, x + 1, y, width, height, oppX, oppY))
        return true;

    if (y > 0 && !board[x][y - 1] &&
            floodFillReachesOpponent(board, x, y - 1, width, height, oppX, oppY))
        return true;

    if (y < height - 1 && !board[x][y + 1] &&
            floodFillReachesOpponent(board, x, y + 1, width, height, oppX, oppY))
            return true;

    return false;
}


bool isOpponentIsolated(const Map &map)
{
    for (Direction dir = DIR_MIN; dir <= DIR_MAX;
            dir = static_cast<Direction>(dir + 1)) {
        if (!map.isWall(dir, SELF)) {
            Map newMap(map);
            newMap.move(dir, SELF);

            int width = newMap.width();
            int height = newMap.height();
            std::vector<std::vector<bool> > board(width, std::vector<bool>(height));

            for (int i = 0; i < width; ++i) {
                for (int j = 0; j < height; ++j) {
                    board[i][j] = newMap.isWall(i, j);
                }
            }
            // allow us to reach the opponent
            board[newMap.enemyX()][newMap.enemyY()] = false;

            if (floodFillReachesOpponent(board, newMap.myX(), newMap.myY(),
                        width, height, newMap.enemyX(), newMap.enemyY()))
            {
                return false;
            }
        }
    }

    return true;
}

Direction decideMoveIsolatedFromOpponent(const Map &map)
{
    std::set<Direction> moves = getPossibleMovesReachableSquares(map);

    for (std::set<Direction>::const_iterator it = moves.begin();
            it != moves.end(); ++it) {
        Map newMap(map);
        newMap.move(*it);

        size_t cnt = 0;
        for (Direction dir = DIR_MIN; dir <= DIR_MAX;
                dir = static_cast<Direction>(dir + 1)) {
            if (newMap.isWall(dir, SELF))
                ++cnt;
        }

        if (cnt >= 2)
            return *it;
    }

    if (moves.empty())
        return NORTH;

    return *moves.begin();
}

