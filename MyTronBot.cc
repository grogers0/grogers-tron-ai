// MyTronBot.cc
//
// This is the code file that you will modify in order to create your entry.

#include "Map.h"
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <set>
#include <iterator>
#include <algorithm>

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

size_t reachableSquares(const Map &map, Player player)
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

std::set<Direction> moveBasedOnReachableSquares(const Map &map)
{
   size_t bestReachable = 0;

    std::set<Direction> bestDirs;

    for (Direction dir = DIR_MIN; dir <= DIR_MAX;
            dir = static_cast<Direction>(dir + 1)) {
        if (!map.isWall(dir, SELF)) {
            Map newMap(map);
            newMap.move(dir, SELF);
            size_t reachable = reachableSquares(newMap, SELF);
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

            if (floodFillReachesOpponent(board, newMap.myX(), newMap.myY(),
                        width, height, newMap.opponentX(), newMap.opponentY()))
            {
                return false;
            }
        }
    }

    return true;
}

std::set<Direction> moveTowardsOpponent(const Map &map)
{
    // don't care what direction the opponent is in if we can't reach them
    if (isOpponentIsolated(map)) {
        fprintf(stderr, "Opponent is isolated, do not move towards them\n");
        return std::set<Direction>();
    }

    fprintf(stderr, "Opponent is reachable, try to move towards them\n");

    int diffX = map.myX() - map.opponentX();
    int diffY = map.myY() - map.opponentY();
    std::set<Direction> ret;

    if (abs(diffX) >= abs(diffY)) {
        if (diffX > 0)
            ret.insert(WEST);
        else if (diffX < 0)
            ret.insert(EAST);
    }

    if (abs(diffX) <= abs(diffY)) {
        if (diffY > 0)
            ret.insert(NORTH);
        else if (diffY < 0)
            ret.insert(SOUTH);
    }

    return ret;
}

Direction whichMove(const Map& map)
{
    int x = map.myX();
    int y = map.myY();

    std::set<Direction> moves1 = moveBasedOnReachableSquares(map);
    std::set<Direction> moves2 = moveTowardsOpponent(map);

    std::vector<Direction> intersection;
    std::set_intersection(moves1.begin(), moves1.end(),
            moves2.begin(), moves2.end(), std::back_inserter(intersection));
    if (intersection.empty())
        return *moves1.begin();

    return *intersection.begin();
}

// Ignore this function. It is just handling boring stuff for you, like
// communicating with the Tron tournament engine.
int main()
{
    Map map;

    while (true)
    {
        map.readFromFile(stdin);
        Direction dir = whichMove(map);
        fprintf(stderr, "Moving %s\n", dirToString(dir));
        Map::sendMoveToServer(dir);
    }
    return 0;
}
