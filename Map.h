// Map.h
//
// Handles the Tron map. Also handles communicating with the Tron game engine.
// You don't need to change anything in this file.

#ifndef MAP_H
#define MAP_H

#include <string>
#include <vector>

enum Direction
{
    NORTH = 1, // negative y direction
    EAST = 2, // positive x direction
    SOUTH = 3, // positive y direction
    WEST = 4, // negative x direction
    DIR_MIN = NORTH,
    DIR_MAX = WEST
};

const char *dirToString(Direction dir);

enum Player
{
    SELF,
    ENEMY
};

inline Player otherPlayer(Player p)
{
    if (p == SELF)
        return ENEMY;
    else
        return SELF;
}

const char *playerToString(Player p);


class Map
{
    public:
        int width() const;
        int height() const;

        bool isWall(int x, int y) const;
        bool isWall(Direction dir, Player p) const;

        void move(Direction dir, Player p);
        void unmove(Direction dir, Player p);

        int cntMoves(Player p) const;

        void print(FILE *) const;

        int myX() const;
        int myY() const;

        int enemyX() const;
        int enemyY() const;

        // Sends your move to the contest engine. The four possible moves are
        //   * 1 -- North. Negative Y direction.
        //   * 2 -- East. Positive X direction.
        //   * 3 -- South. Positive X direction.
        //   * 4 -- West. Negative X direction.
        static void sendMoveToServer(Direction move);

        // Load a board from an open file handle. To read from the console,
        // pass stdin, which is actually a (FILE*).  file_handle -- an open
        // file handle from which to read.
        //
        // If there is a problem, the function returns NULL. Otherwise, a valid
        // Board structure is returned.
        //
        // The file should be an ascii file. The first line contains the width
        // and height of the board, separated by a space. subsequent lines
        // contain visual representations of the rows of the board, using '#'
        // and space characters.  The starting positions of the two players are
        // indicated by '1' and '2' characters. There must be exactly one '1'
        // character and one '2' character on the board. For example:
        // 6 4
        // ######
        // #1# 2#
        // #   ##
        // ######
        void readFromFile(FILE *file_handle);

    private:
        // Indicates whether or not each cell in the board is passable.
        std::vector<bool> is_wall;

        // The locations of both players.
        int player_one_x, player_one_y;
        int player_two_x, player_two_y;

        // Map dimensions.
        int map_width, map_height;
};

// Returns the width of the Tron map.
inline int Map::width() const { return map_width; }

// Returns the height of the Tron map.
inline int Map::height() const { return map_height; }

// Returns whether or not the given cell is a wall or not. TRUE means it's
// a wall, FALSE means it's not a wall, and is passable. Any spaces that are
// not on the board are deemed to be walls.
inline bool Map::isWall(int x, int y) const
{
    return is_wall[x*map_height + y];
}

inline bool Map::isWall(Direction dir, Player p) const
{
    int x, y;
    switch (p) {
        case SELF:
            x = player_one_x;
            y = player_one_y;
            break;
        case ENEMY:
            x = player_two_x;
            y = player_two_y;
            break;
        default:
            return false;
    }

    switch (dir) {
        case NORTH:
            return isWall(x, y - 1);
        case SOUTH:
            return isWall(x, y + 1);
        case WEST:
            return isWall(x - 1, y);
        case EAST:
            return isWall(x + 1, y);
        default:
            return false;
    }
}

inline void Map::move(Direction dir, Player p)
{
    int *x, *y;
    switch (p) {
        case SELF:
            x = &player_one_x;
            y = &player_one_y;
            break;
        case ENEMY:
            x = &player_two_x;
            y = &player_two_y;
            break;
        default:
            return;
    }

    is_wall[(*x)*map_height + (*y)] = true;

    switch (dir) {
        case NORTH: (*y)--; break;
        case SOUTH: (*y)++; break;
        case WEST: (*x)--; break;
        case EAST: (*x)++; break;
        default: return;
    }

    if (p != SELF) {
        is_wall[player_one_x*map_height + player_one_y] = true;
        is_wall[player_two_x*map_height + player_two_y] = true;
    }
}


inline void Map::unmove(Direction dir, Player p)
{
    int *x, *y;
    switch (p) {
        case SELF:
            x = &player_one_x;
            y = &player_one_y;
            break;
        case ENEMY:
            x = &player_two_x;
            y = &player_two_y;
            break;
        default:
            return;
    }

    is_wall[(*x)*map_height + (*y)] = false;

    // unmove is backwards the regular move...
    switch (dir) {
        case NORTH: (*y)++; break;
        case SOUTH: (*y)--; break;
        case WEST: (*x)++; break;
        case EAST: (*x)--; break;
        default: return;
    }

    if (p != SELF) {
        is_wall[player_one_x*map_height + player_one_y] = false;
        is_wall[player_two_x*map_height + player_two_y] = false;
    }
}

inline int Map::cntMoves(Player p) const
{
    int cnt = 0;
    for (Direction dir = DIR_MIN; dir <= DIR_MAX;
            dir = static_cast<Direction>(dir + 1)) {
        if (!isWall(dir, p))
            ++cnt;
    }
    return cnt;
}

// Get my X and Y position. These are zero-based.
inline int Map::myX() const { return player_one_x; }
inline int Map::myY() const { return player_one_y; }

// Get the opponent's X and Y position. These are zero-based.
inline int Map::enemyX() const { return player_two_x; }
inline int Map::enemyY() const { return player_two_y; }

#endif
