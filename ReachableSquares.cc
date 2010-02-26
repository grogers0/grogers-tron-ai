#include "MoveDeciders.h"
#include <cstdio>
#include <cassert>
#include <map>
#include <utility>

static int countReachable(const std::vector<bool> &boardIn, int px, int py,
        std::map<std::pair<int, int>, int> &);

static inline int cntMovesFromSquare(const std::vector<bool> &board,
        int x, int y)
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

static int floodFill(std::vector<bool> &board, int x, int y)
{
    int ret = 1;

    board[x*height + y] = true;

    if (!board[(x - 1)*height + y])
        ret += floodFill(board, x - 1, y);

    if (!board[(x + 1)*height + y])
        ret += floodFill(board, x + 1, y);

    if (!board[x*height + (y - 1)])
        ret += floodFill(board, x, y - 1);

    if (!board[x*height + (y + 1)])
        ret += floodFill(board, x, y + 1);

    return ret;
}

static int floodFillCorr(std::vector<bool> &board, int x, int y, int &extra,
        std::map<std::pair<int, int>, int> &corridorEntrances)
{
    int ret = 1;

    board[x*height + y] = true;

    {
        std::map<std::pair<int, int>, int>::iterator it =
            corridorEntrances.find(std::make_pair(x, y));
        if (it != corridorEntrances.end()) {
            //fprintf(stderr, "entrance x: %d, y: %d, cnt: %d\n", x, y, it->second);
            if (it->second > extra)
                extra = it->second;
            corridorEntrances.erase(it);
        }
    }

    if (!board[(x - 1)*height + y])
        ret += floodFillCorr(board, x - 1, y, extra, corridorEntrances);

    if (!board[(x + 1)*height + y])
        ret += floodFillCorr(board, x + 1, y, extra, corridorEntrances);

    if (!board[x*height + (y - 1)])
        ret += floodFillCorr(board, x, y - 1, extra, corridorEntrances);

    if (!board[x*height + (y + 1)])
        ret += floodFillCorr(board, x, y + 1, extra, corridorEntrances);

    return ret;
}

void fillUnreachableSquares(std::vector<bool> &board, int x, int y)
{
    std::vector<bool> boardReachFilled(board);

    floodFill(boardReachFilled, x, y);

    for (int i = 0; i < width*height; ++i) {
        if (!board[i] && !boardReachFilled[i])
            board[i] = true;
    }
}

static void visitSquareForPruning(std::vector<bool> &board, int x, int y,
        int px, int py,
        std::map<std::pair<int, int>, int> &corridorEntrances);

bool isCorridorSquare(const std::vector<bool> &board, int x, int y)
{
    int x2, y2, xmoves = 0, ymoves = 0;
    if (!board[(x + 1)*height + y]) {
        x2 = x + 1;
        ++xmoves;
    }
    if (!board[(x - 1)*height + y]) {
        x2 = x - 1;
        ++xmoves;
    }
    if (!board[x*height + y + 1]) {
        y2 = y + 1;
        ++ymoves;
    }
    if (!board[x*height + y - 1]) {
        y2 = y - 1;
        ++ymoves;
    }

    return (xmoves == 0 && ymoves == 2) ||
        (xmoves == 2 && ymoves == 0) ||
        (xmoves == 1 && ymoves == 1 && board[x2*height + y2]);
}

static std::vector<bool> notCorridors;

static void markHallwayNotCorridor(std::vector<bool> &board, int x, int y)
{
    if (cntMovesFromSquare(board, x, y) != 1)
        return;

    notCorridors[x*height + y] = true;

    int x2 = x, y2 = y;
    if (!board[(x + 1)*height + y])
        x2 = x + 1;
    else if (!board[(x - 1)*height + y])
        x2 = x - 1;
    else if (!board[x*height + y + 1])
        y2 = y + 1;
    else if (!board[x*height + y - 1])
        y2 = y - 1;

    board[x*height + y] = true;
    markHallwayNotCorridor(board, x2, y2);
    board[x*height + y] = false;
}

static void pruneOneCorridor(std::vector<bool> &board, int x, int y,
        int px, int py,
        std::map<std::pair<int, int>, int> &corridorEntrances)
{
    if (notCorridors[x*height + y])
        return;

    if (!isCorridorSquare(board, x, y)) {
        return;
    }

    int xn[2] = {x, x}, yn[2] = {y, y}, n = 0;
    if (!board[(x + 1)*height + y])
        xn[n++] = x + 1;
    if (!board[(x - 1)*height + y])
        xn[n++] = x - 1;
    if (!board[x*height + y + 1])
        yn[n++] = y + 1;
    if (!board[x*height + y - 1])
        yn[n++] = y - 1;

    board[x*height + y] = true;

    if (squaresReachEachOther(board, xn[0], yn[0], xn[1], yn[1])) {
        notCorridors[x*height + y] = true;

        markHallwayNotCorridor(board, xn[0], yn[0]);
        markHallwayNotCorridor(board, xn[1], yn[1]);

        board[x*height + y] = false;
        return;
    }

    int pside, cside;
    if (squaresReachEachOther(board, xn[0], yn[0], px, py)) {
        pside = 0;
        cside = 1;
    } else {
        pside = 1;
        cside = 0;
    }

    int cnt = 1;

    // step out of the corridor on the closed side
    while (cntMovesFromSquare(board, xn[cside], yn[cside]) == 1) {
        board[xn[cside]*height + yn[cside]] = true;
        if (!board[(xn[cside] + 1)*height + yn[cside]])
            xn[cside] = xn[cside] + 1;
        else if (!board[(xn[cside] - 1)*height + yn[cside]])
            xn[cside] = xn[cside] - 1;
        else if (!board[xn[cside]*height + yn[cside] + 1])
            yn[cside] = yn[cside] + 1;
        else if (!board[xn[cside]*height + yn[cside] - 1])
            yn[cside] = yn[cside] - 1;

        ++cnt;
    }

    //fprintf(stderr, "corridor x: %d, y: %d, cnt: %d\n", x, y, cnt);

    cnt += countReachable(board, xn[cside], yn[cside], corridorEntrances);

    {
        std::map<std::pair<int, int>, int>::iterator it =
            corridorEntrances.find(std::make_pair(x, y));
        if (it != corridorEntrances.end()) {
            if (it->second > cnt)
                cnt = it->second;
            corridorEntrances.erase(it);
        }
    }

    std::map<std::pair<int, int>, int>::iterator it =
        corridorEntrances.find(std::make_pair(xn[pside], yn[pside]));
    if (it == corridorEntrances.end())
        corridorEntrances.insert(std::make_pair(std::make_pair(xn[pside], yn[pside]), cnt));
    else if (cnt > it->second)
        it->second = cnt;

    visitSquareForPruning(board, xn[pside], yn[pside], px, py, corridorEntrances);
}

static void pruneOneDeadEnd(std::vector<bool> &board, int x, int y,
        int px, int py,
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

    visitSquareForPruning(board, x2, y2, px, py, corridorEntrances);
}

static void visitSquareForPruning(std::vector<bool> &board, int x, int y,
        int px, int py,
        std::map<std::pair<int, int>, int> &corridorEntrances)
{
    if (board[x*height + y])
        return;

    // player square can't be a corridor
    if (x == px && y == py)
        return;

    int moves = cntMovesFromSquare(board, x, y);
    if (moves == 1)
        pruneOneDeadEnd(board, x, y, px, py, corridorEntrances);
    else if (moves == 2)
        pruneOneCorridor(board, x, y, px, py, corridorEntrances);
}

static void pruneCorridors(std::vector<bool> &board, int px, int py,
        std::map<std::pair<int, int>, int> &corridorEntrances)
{
    notCorridors.clear();
    notCorridors.resize(width*height);

    for (int i = 1; i < width - 1; ++i) {
        for (int j = 1; j < height - 1; ++j) {
            visitSquareForPruning(board, i, j, px, py, corridorEntrances);
        }
    }
}

static int countReachable(const std::vector<bool> &boardIn, int px, int py,
        std::map<std::pair<int, int>, int> &corridorEntrances)
{
    std::vector<bool> board(boardIn);

    board[px*height + py] = false;

    fillUnreachableSquares(board, px, py);

    pruneCorridors(board, px, py, corridorEntrances);

    int corridorDepth = 0;

    int fill = floodFillCorr(board, px, py, corridorDepth, corridorEntrances);

    //fprintf(stderr, "reachable px: %d, py: %d, fill: %d\n", px, py, fill + corridorDepth);

    return fill + corridorDepth;
}

int countReachableSquares(const Map &map, Player player)
{
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

    std::map<std::pair<int, int>, int> corridorEntrances;

    return countReachable(map.getBoard(), x, y, corridorEntrances) - 1;
}
