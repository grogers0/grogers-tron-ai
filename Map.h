#ifndef MAP_H
#define MAP_H

#include <string>
#include <vector>
#include <stdint.h>

#include "position.h"

enum Direction
{
    NORTH,
    SOUTH,
    WEST,
    EAST,

    DIR_MIN = NORTH,
    DIR_MAX = EAST
};

const char *dirToString(Direction dir);

enum Player
{
    SELF,
    ENEMY
};

const char *playerToString(Player p);

extern int width, height; // width and height never change, so why bother passing them around

inline
int index(position p)
{
    return p.x * height + p.y;
}

void zobrist_init();
typedef uint64_t HASH_TYPE;

const int TRANSPOSITION_TABLE_SIZE = 1 * 1024 * 1024;

class TranspositionTable
{
    public:
        struct Entry
        {
            int heuristic;
        };

        const Entry *get(HASH_TYPE hash);
        void set(HASH_TYPE, const Entry &entry);

    private:
        std::pair<HASH_TYPE, Entry> data[TRANSPOSITION_TABLE_SIZE];
};

extern TranspositionTable trans_table;

class Map
{
    public:
        bool isWall(position) const;
        bool isWall(Direction dir, Player p) const;

        void move(Direction myDir, Player p);
        void unmove(Direction myDir, Player p);

        int cntMoves(Player p) const;

        const std::vector<bool> &getBoard() const;

        void print(FILE *) const;

        position my_pos() const;
        position enemy_pos() const;

        HASH_TYPE hash() const;

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
        bool readFromFile(FILE *file_handle);

    private:
        // Indicates whether or not each cell in the board is passable.
        std::vector<bool> is_wall;

        position player_pos[2];
        HASH_TYPE current_hash;
};

// Returns whether or not the given cell is a wall or not. TRUE means it's
// a wall, FALSE means it's not a wall, and is passable. Any spaces that are
// not on the board are deemed to be walls.
inline
bool Map::isWall(position pos) const
{
    return is_wall[index(pos)];
}

inline
bool Map::isWall(Direction dir, Player p) const
{
    position pos;
    switch (p) {
        case SELF:
            pos = player_pos[0];
            break;
        case ENEMY:
            pos = player_pos[1];
            break;
        default:
            return false;
    }

    switch (dir) {
        case NORTH: return is_wall[index(pos.north())];
        case SOUTH: return is_wall[index(pos.south())];
        case WEST: return is_wall[index(pos.west())];
        case EAST: return is_wall[index(pos.east())];
        default: return false;
    }
}



inline
int Map::cntMoves(Player p) const
{
    int cnt = 0;
    for (Direction dir = DIR_MIN; dir <= DIR_MAX;
            dir = static_cast<Direction>(dir + 1)) {
        if (!isWall(dir, p))
            ++cnt;
    }
    return cnt;
}

inline
const std::vector<bool> &Map::getBoard() const
{
    return is_wall;
}

inline
int cntMovesFromSquare(const std::vector<bool> &board, position pos)
{
    int cnt = 0;
    if (!board[index(pos.north())])
        ++cnt;
    if (!board[index(pos.south())])
        ++cnt;
    if (!board[index(pos.west())])
        ++cnt;
    if (!board[index(pos.east())])
        ++cnt;
    return cnt;
}

inline
position Map::my_pos() const
{
    return player_pos[0];
}

inline
position Map::enemy_pos() const
{
    return player_pos[1];
}

inline HASH_TYPE Map::hash() const
{
    return current_hash;
}

#endif
