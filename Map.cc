#include "Map.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include <unistd.h>
#include <fcntl.h>

int width, height;

const char *dirToString(Direction dir)
{
    switch (dir) {
        case NORTH: return "North";
        case SOUTH: return "South";
        case WEST: return "West";
        case EAST: return "East";
        default: return "Unknown Direction";
    }
}

const char *playerToString(Player p)
{
    switch (p) {
        case SELF: return "Self";
        case ENEMY: return "Enemy";
        default: return "Unknown Player";
    }
}


std::vector<HASH_TYPE> wall_hashes;
std::vector<HASH_TYPE> player_hashes[2];

void zobrist_init()
{
    wall_hashes.resize(width*height);
    player_hashes[0].resize(width*height);
    player_hashes[1].resize(width*height);

    int fd = open("/dev/urandom", O_RDONLY);

    if (fd == -1) {
        perror("failed to open /dev/urandom for reading");
        exit(1);
    }

    read(fd, &wall_hashes.front(), width*height*sizeof(HASH_TYPE));
    read(fd, &player_hashes[0].front(), width*height*sizeof(HASH_TYPE));
    read(fd, &player_hashes[1].front(), width*height*sizeof(HASH_TYPE));

    close(fd);
}

TranspositionTable trans_table;

const TranspositionTable::Entry *TranspositionTable::get(HASH_TYPE hash)
{
    if (data[hash % TRANSPOSITION_TABLE_SIZE].first == hash)
        return &data[hash % TRANSPOSITION_TABLE_SIZE].second;
    else
        return NULL;
}

void TranspositionTable::set(HASH_TYPE hash, const Entry &entry)
{
    data[hash % TRANSPOSITION_TABLE_SIZE].first = hash;
    data[hash % TRANSPOSITION_TABLE_SIZE].second = entry;
}


void Map::move(Direction dir, Player p)
{
    position currpos, nextpos;

    switch (p) {
        case SELF: currpos = player_pos[0]; break;
        case ENEMY: currpos = player_pos[1]; break;
    }

    switch (dir) {
        case NORTH: nextpos = currpos.north(); break;
        case SOUTH: nextpos = currpos.south(); break;
        case WEST: nextpos = currpos.west(); break;
        case EAST: nextpos = currpos.east(); break;
    }

    is_wall[index(nextpos)] = true;

    switch (p) {
        case SELF: player_pos[0] = nextpos; break;
        case ENEMY: player_pos[1] = nextpos; break;
    }

    current_hash ^= player_hashes[p][index(currpos)];
    current_hash ^= player_hashes[p][index(nextpos)];
    current_hash ^= wall_hashes[index(nextpos)];
}

void Map::unmove(Direction dir, Player p)
{
    position currpos, nextpos;

    switch (p) {
        case SELF: currpos = player_pos[0]; break;
        case ENEMY: currpos = player_pos[1]; break;
    }

    // backwards... (we are unmoving)
    switch (dir) {
        case NORTH: nextpos = currpos.south(); break;
        case SOUTH: nextpos = currpos.north(); break;
        case WEST: nextpos = currpos.east(); break;
        case EAST: nextpos = currpos.west(); break;
    }

    is_wall[index(currpos)] = false;

    switch (p) {
        case SELF: player_pos[0] = nextpos; break;
        case ENEMY: player_pos[1] = nextpos; break;
    }

    current_hash ^= player_hashes[p][index(currpos)];
    current_hash ^= player_hashes[p][index(nextpos)];
    current_hash ^= wall_hashes[index(currpos)];
}


void Map::print(FILE *fp) const
{
    position pos;
    fprintf(stderr, "%d %d\n", width, height);
    for (pos.y = 0; pos.y < height; ++pos.y) {
        for (pos.x = 0; pos.x < width; ++pos.x) {
            if (pos == player_pos[0])
                fprintf(fp, "1");
            else if (pos == player_pos[1])
                fprintf(fp, "2");
            else if (isWall(pos))
                fprintf(fp, "#");
            else
                fprintf(fp, " ");
        }

        fprintf(fp, "\n");
    }
}

bool Map::readFromFile(FILE *file_handle)
{
    int c;
    int num_items = fscanf(file_handle, "%d %d\n", &width, &height);
    if (feof(file_handle) || num_items < 2) {
        return false;
    }

    static bool first_time = true;
    if (first_time) {
        zobrist_init();
        first_time = false;
    }

    current_hash = 0;

    is_wall = std::vector<bool>(width*height, false);
    position pos(0, 0);
    while (pos.y < height && (c = fgetc(file_handle)) != EOF) {
        switch (c) {
            case '\r':
                break;
            case '\n':
                if (pos.x != width) {
                    fprintf(stderr, "x != width in Board_ReadFromStream\n");
                    return false;
                }
                ++pos.y;
                pos.x = 0;
                break;
            case '#':
                if (pos.x >= width) {
                    fprintf(stderr, "x >= width in Board_ReadFromStream\n");
                    return false;
                }
                is_wall[index(pos)] = true;
                current_hash ^= wall_hashes[index(pos)];
                ++pos.x;
                break;
            case ' ':
                if (pos.x >= width) {
                    fprintf(stderr, "x >= width in Board_ReadFromStream\n");
                    return false;
                }
                is_wall[index(pos)] = false;
                ++pos.x;
                break;
            case '1':
                if (pos.x >= width) {
                    fprintf(stderr, "x >= width in Board_ReadFromStream\n");
                    return false;
                }
                is_wall[index(pos)] = true;
                player_pos[0] = pos;
                current_hash ^= wall_hashes[index(pos)];
                current_hash ^= player_hashes[0][index(pos)];
                ++pos.x;
                break;
            case '2':
                if (pos.x >= width) {
                    fprintf(stderr, "x >= width in Board_ReadFromStream\n");
                    return false;
                }
                is_wall[index(pos)] = true;
                player_pos[1] = pos;
                current_hash ^= wall_hashes[index(pos)];
                current_hash ^= player_hashes[1][index(pos)];
                ++pos.x;
                break;
            default:
                fprintf(stderr, "unexpected character %d in Board_ReadFromStream", c);
                return false;
        }
    }

    return true;
}
