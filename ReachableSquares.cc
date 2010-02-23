#include "MoveDeciders.h"
#include <cstdio>
#include <cassert>
#include <map>
#include <utility>

static inline int cntMovesFromSquare(const std::vector<bool> &board,
        int x, int y, int height)
{
    int cnt = 0;
    if (!board[(x - 1)*height + y])
        ++cnt;
    if (!board[(x + 1)*height + y])
        ++cnt;
    if (!board[x*height + (y - 1)])
        ++cnt;
    if (!board[x*height + (y + 1)])
        ++cnt;
    return cnt;
}

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

static void fillUnreachableSquares(std::vector<bool> &board, int x, int y,
        int width, int height)
{
    std::vector<bool> boardReachFilled(board);

    floodFill(boardReachFilled, x, y, width, height);

    for (int i = 0; i < width*height; ++i) {
        if (!board[i] && !boardReachFilled[i])
            board[i] = true;
    }
}

static void visitSquareForPruning(std::vector<bool> &board, int x, int y,
        int px, int py, int height,
        std::map<std::pair<int, int>, int> &corridorEntrances);

static void pruneOneDeadEnd(std::vector<bool> &board, int x, int y,
        int px, int py, int height,
        std::map<std::pair<int, int>, int> &corridorEntrances)
{
    int cnt = 1;

    {
        std::map<std::pair<int, int>, int>::iterator it =
            corridorEntrances.find(std::make_pair(x, y));
        if (it != corridorEntrances.end()) {
            cnt = it->second + 1;
            corridorEntrances.erase(it);
        }
    }

    int x2 = x, y2 = y;
    if (!board[(x + 1)*height + y])
        x2 = x + 1;
    else if (!board[(x - 1)*height + y])
        x2 = x - 1;
    else if (!board[x*height + y + 1])
        y2 = y + 1;
    else if (!board[x*height + y - 1])
        y2 = y - 1;
    else
        assert(false);

    board[x*height + y] = true;

    std::map<std::pair<int, int>, int>::iterator it =
        corridorEntrances.find(std::make_pair(x2, y2));
    if (it == corridorEntrances.end())
        corridorEntrances.insert(std::make_pair(std::make_pair(x2, y2), cnt));
    else if (cnt > it->second)
        it->second = cnt;

    visitSquareForPruning(board, x2, y2, px, py, height, corridorEntrances);
}

static void visitSquareForPruning(std::vector<bool> &board, int x, int y,
        int px, int py, int height,
        std::map<std::pair<int, int>, int> &corridorEntrances)
{
    if (board[x*height + y])
        return;

    // player square can't be a corridor
    if (x == px && y == py)
        return;

    int moves = cntMovesFromSquare(board, x, y, height);
    if (moves == 1)
        pruneOneDeadEnd(board, x, y, px, py, height, corridorEntrances);
}

static int pruneCorridors(std::vector<bool> &board, int px, int py,
        int width, int height)
{
    std::map<std::pair<int, int>, int> corridorEntrances;

    for (int i = 1; i < width - 1; ++i) {
        for (int j = 1; j < height - 1; ++j) {
            visitSquareForPruning(board, i, j, px, py, height, corridorEntrances);
        }
    }

    int cnt = 0;
    for (std::map<std::pair<int, int>, int>::const_iterator it =
            corridorEntrances.begin(); it != corridorEntrances.end(); ++it) {
        if (it->second > cnt)
            cnt = it->second;
    }

    return cnt;
}

int countReachableSquares(const Map &map, Player player)
{
    int width = map.width();
    int height = map.height();

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

    std::vector<bool> board(map.getBoard());

    board[map.myX()*height + map.myY()] = false;
    board[map.enemyX()*height + map.enemyY()] = false;

    fillUnreachableSquares(board, x, y, width, height);

    int corridorDepth = pruneCorridors(board, x, y, width, height);

    return floodFill(board, x, y, width, height) - 1 + corridorDepth;
}
