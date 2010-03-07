#include "MoveDeciders.h"
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <map>
#include <utility>

static int countReachable(const std::vector<bool> &boardIn,
         position pos, std::map<position, int> &);

static int floodFill(std::vector<bool> &board, position pos)
{
    int ret = 1;

    board[index(pos)] = true;

    if (!board[index(pos.north())])
        ret += floodFill(board, pos.north());

    if (!board[index(pos.south())])
        ret += floodFill(board, pos.south());

    if (!board[index(pos.west())])
        ret += floodFill(board, pos.west());

    if (!board[index(pos.east())])
        ret += floodFill(board, pos.east());

    return ret;
}

static inline int checkerSign(position pos)
{
    if ((pos.x%2 == 0) != (pos.y%2 == 0))
        return 1;
    else
        return -1;
}

static int floodFillCorr(std::vector<bool> &board, position pos, int &extra,
        int &checkerDiff, int &signCorr,
        std::map<position, int> &corridorEntrances)
{
    int ret = 1;

    board[index(pos)] = true;

    checkerDiff += checkerSign(pos);

    {
        std::map<position, int>::iterator it = corridorEntrances.find(pos);
        if (it != corridorEntrances.end()) {
            //fprintf(stderr, "entrance x: %d, y: %d, cnt: %d\n", x, y, it->second);
            if (it->second > extra) {
                extra = it->second;
                signCorr = checkerSign(pos);
            }
            corridorEntrances.erase(it);
        }
    }

    if (!board[index(pos.north())])
        ret += floodFillCorr(board, pos.north(), extra, checkerDiff, signCorr, corridorEntrances);

    if (!board[index(pos.south())])
        ret += floodFillCorr(board, pos.south(), extra, checkerDiff, signCorr, corridorEntrances);

    if (!board[index(pos.west())])
        ret += floodFillCorr(board, pos.west(), extra, checkerDiff, signCorr, corridorEntrances);

    if (!board[index(pos.east())])
        ret += floodFillCorr(board, pos.east(), extra, checkerDiff, signCorr, corridorEntrances);

    return ret;
}

void fillUnreachableSquares(std::vector<bool> &board, position pos)
{
    std::vector<bool> boardReachFilled(board);

    floodFill(boardReachFilled, pos);

    for (int i = 0; i < width*height; ++i) {
        if (!board[i] && !boardReachFilled[i])
            board[i] = true;
    }
}

static void visitSquareForPruning(std::vector<bool> &board, position pos,
        position player_pos, std::map<position, int> &corridorEntrances);

// note it must be checked before entry if this square is not a wall or a dead end
bool isCorridorSquare(const std::vector<bool> &board, position pos)
{
    if (board[index(pos.north())] && board[index(pos.south())])
        return true;
    if (board[index(pos.west())] && board[index(pos.east())])
        return true;
    if (board[index(pos.north().east())] && board[index(pos.south().west())])
        return true;
    // hmmm, this was a bug in the old version but fixing the bug seems
    // to make it dummer, so oh well...
    //if (board[index(pos.north().west())] && board[index(pos.south().east())])
        //return true;

    return false;
}

static std::vector<bool> notCorridors;

static void markHallwayNotCorridor(std::vector<bool> &board, position pos)
{
    if (cntMovesFromSquare(board, pos) != 1)
        return;

    notCorridors[index(pos)] = true;

    position pos2;
    if (!board[index(pos.north())])
        pos2 = pos.north();
    else if (!board[index(pos.south())])
        pos2 = pos.south();
    else if (!board[index(pos.west())])
        pos2 = pos.west();
    else if (!board[index(pos.east())])
        pos2 = pos.east();

    board[index(pos)] = true;
    markHallwayNotCorridor(board, pos2);
    board[index(pos)] = false;
}

static void pruneOneCorridor(std::vector<bool> &board, position pos,
        position player_pos, std::map<position, int> &corridorEntrances)
{
    if (notCorridors[index(pos)])
        return;

    if (!isCorridorSquare(board, pos)) {
        return;
    }

    position posn[2] = {pos, pos};
    int n = 0;
    if (!board[index(pos.north())])
        posn[n++] = pos.north();
    if (!board[index(pos.south())])
        posn[n++] = pos.south();
    if (!board[index(pos.west())])
        posn[n++] = pos.west();
    if (!board[index(pos.east())])
        posn[n++] = pos.east();
    assert(n == 2);

    board[index(pos)] = true;

    if (squaresReachEachOther(board, posn[0], posn[1])) {
        notCorridors[index(pos)] = true;

        markHallwayNotCorridor(board, posn[0]);
        markHallwayNotCorridor(board, posn[1]);

        board[index(pos)] = false;
        return;
    }

    int pside, cside;
    if (squaresReachEachOther(board, posn[0], player_pos)) {
        pside = 0;
        cside = 1;
    } else {
        pside = 1;
        cside = 0;
    }

    int cnt = 1;

    // step out of the corridor on the closed side
    while (cntMovesFromSquare(board, posn[cside]) == 1) {
        board[index(posn[cside])] = true;

        if (!board[index(posn[cside].north())])
            posn[cside] = posn[cside].north();
        else if (!board[index(posn[cside].south())])
            posn[cside] = posn[cside].south();
        else if (!board[index(posn[cside].west())])
            posn[cside] = posn[cside].west();
        else if (!board[index(posn[cside].east())])
            posn[cside] = posn[cside].east();

        ++cnt;
    }

    //fprintf(stderr, "corridor x: %d, y: %d, cnt: %d\n", x, y, cnt);

    cnt += countReachable(board, posn[cside], corridorEntrances);

    {
        std::map<position, int>::iterator it = corridorEntrances.find(pos);
        if (it != corridorEntrances.end()) {
            if (it->second > cnt)
                cnt = it->second;
            corridorEntrances.erase(it);
        }
    }

    std::map<position, int>::iterator it = corridorEntrances.find(posn[pside]);
    if (it == corridorEntrances.end())
        corridorEntrances.insert(std::make_pair(posn[pside], cnt));
    else if (cnt > it->second)
        it->second = cnt;

    visitSquareForPruning(board, posn[pside], player_pos, corridorEntrances);
}

static void pruneOneDeadEnd(std::vector<bool> &board, position pos,
        position player_pos, std::map<position, int> &corridorEntrances)
{
    int cnt = 1;

    {
        std::map<position, int>::iterator it = corridorEntrances.find(pos);
        if (it != corridorEntrances.end()) {
            cnt = it->second + 1;
            corridorEntrances.erase(it);
        }
    }

    position pos2;
    if (!board[index(pos.north())])
        pos2 = pos.north();
    else if (!board[index(pos.south())])
        pos2 = pos.south();
    else if (!board[index(pos.west())])
        pos2 = pos.west();
    else if (!board[index(pos.east())])
        pos2 = pos.east();
    else
        assert(false);

    board[index(pos)] = true;

    std::map<position, int>::iterator it = corridorEntrances.find(pos2);
    if (it == corridorEntrances.end())
        corridorEntrances.insert(std::make_pair(pos2, cnt));
    else if (cnt > it->second)
        it->second = cnt;

    visitSquareForPruning(board, pos2, player_pos, corridorEntrances);
}

static void visitSquareForPruning(std::vector<bool> &board, position pos,
        position player_pos, std::map<position, int> &corridorEntrances)
{
    if (board[index(pos)])
        return;

    // player square can't be a corridor
    if (pos == player_pos)
        return;

    int moves = cntMovesFromSquare(board, pos);
    if (moves == 1)
        pruneOneDeadEnd(board, pos, player_pos, corridorEntrances);
    else if (moves == 2)
        pruneOneCorridor(board, pos, player_pos, corridorEntrances);
}

static void pruneCorridors(std::vector<bool> &board, position player_pos,
        std::map<position, int> &corridorEntrances)
{
    notCorridors.clear();
    notCorridors.resize(width*height);

    position pos;
    for (pos.x = 1; pos.x < width - 1; ++pos.x) {
        for (pos.y = 1; pos.y < height - 1; ++pos.y) {
            visitSquareForPruning(board, pos, player_pos, corridorEntrances);
        }
    }
}

static int countReachable(const std::vector<bool> &boardIn, position player_pos,
        std::map<position, int> &corridorEntrances)
{
    std::vector<bool> board(boardIn);

    board[index(player_pos)] = false;

    fillUnreachableSquares(board, player_pos);

    pruneCorridors(board, player_pos, corridorEntrances);

    int corridorDepth = 0;
    int checkerDiff = 0;
    int corridorSign = 0;

    int fill = floodFillCorr(board, player_pos, corridorDepth, checkerDiff,
            corridorSign, corridorEntrances);

    int checkerSub = 0;
    int playerSign = checkerSign(player_pos);
    if (corridorSign == 0) { // no corridors
        if (checkerDiff == 0)
            checkerSub = 0;
        else if (playerSign * checkerDiff > 0)
            checkerSub = abs(checkerDiff);
        else
            checkerSub = abs(checkerDiff) - 1;
    } else {
        if (playerSign * corridorSign < 0)
            checkerSub = abs(checkerDiff);
        else if (playerSign * checkerDiff > 0)
            checkerSub = abs(checkerDiff) - 1;
        else
            checkerSub = abs(checkerDiff) + 1;
    }

    //fprintf(stderr, "reachable px: %d, py: %d, fill: %d\n", px, py, fill + corridorDepth);

    //fprintf(stderr, "checker diff: %d, psign: %d, corrsign: %d, sub: %d\n", checkerDiff, playerSign, corridorSign, checkerSub);
    return fill + corridorDepth - checkerSub;
}

int countReachableSquares(const Map &map, Player player)
{
    position pos;
    switch (player) {
        case SELF: pos = map.my_pos(); break;
        case ENEMY: pos = map.enemy_pos(); break;
    }

    std::map<position, int> corridorEntrances;

    return countReachable(map.getBoard(), pos, corridorEntrances) - 1;
}
