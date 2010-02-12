// Map.cc

#include "Map.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

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

Player otherPlayer(Player p)
{
    if (p == SELF)
        return ENEMY;
    else
        return SELF;
}

const char *playerToString(Player p)
{
    switch (p) {
        case SELF: return "Self";
        case ENEMY: return "Enemy";
        default: return "Unknown Player";
    }
}


Map::Map() :
    player_one_x(0),
    player_one_y(0),
    player_two_x(0),
    player_two_y(0),
    map_width(0),
    map_height(0)
{
}

bool Map::isWall(Direction dir, Player p) const
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
    }
}

void Map::move(Direction dir, Player p, bool halfMove)
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
    }

    switch (dir) {
        case NORTH: (*y)--; break;
        case SOUTH: (*y)++; break;
        case WEST: (*x)--; break;
        case EAST: (*x)++; break;
    }

    if (!halfMove) {
        is_wall[player_one_x*map_height + player_one_y] = true;
        is_wall[player_two_x*map_height + player_two_y] = true;
    }
}

void Map::print() const
{
    for (int y = 0; y < map_height; ++y) {
        for (int x = 0; x < map_width; ++x) {
            if (player_one_x == x && player_one_y == y)
                fprintf(stderr, "1");
            else if (player_two_x == x && player_two_y == y)
                fprintf(stderr, "2");
            else if (isWall(x, y))
                fprintf(stderr, "#");
            else
                fprintf(stderr, " ");
        }

        fprintf(stderr, "\n");
    }
}

void Map::sendMoveToServer(Direction move)
{
    fprintf(stdout, "%d\n", move);
    fflush(stdout);
}

void Map::readFromFile(FILE *file_handle)
{
    int x, y, c;
    int num_items = fscanf(file_handle, "%d %d\n", &map_width, &map_height);
    if (feof(file_handle) || num_items < 2) {
        exit(0); // End of stream means end of game. Just exit.
    }
    is_wall = std::vector<bool>(map_width*map_height, false);
    x = 0;
    y = 0;
    while (y < map_height && (c = fgetc(file_handle)) != EOF) {
        switch (c) {
            case '\r':
                break;
            case '\n':
                if (x != map_width) {
                    fprintf(stderr, "x != width in Board_ReadFromStream\n");
                    return;
                }
                ++y;
                x = 0;
                break;
            case '#':
                if (x >= map_width) {
                    fprintf(stderr, "x >= width in Board_ReadFromStream\n");
                    return;
                }
                is_wall[x*map_height + y] = true;
                ++x;
                break;
            case ' ':
                if (x >= map_width) {
                    fprintf(stderr, "x >= width in Board_ReadFromStream\n");
                    return;
                }
                is_wall[x*map_height + y] = false;
                ++x;
                break;
            case '1':
                if (x >= map_width) {
                    fprintf(stderr, "x >= width in Board_ReadFromStream\n");
                    return;
                }
                is_wall[x*map_height + y] = true;
                player_one_x = x;
                player_one_y = y;
                ++x;
                break;
            case '2':
                if (x >= map_width) {
                    fprintf(stderr, "x >= width in Board_ReadFromStream\n");
                    return;
                }
                is_wall[x*map_height + y] = true;
                player_two_x = x;
                player_two_y = y;
                ++x;
                break;
            default:
                fprintf(stderr, "unexpected character %d in Board_ReadFromStream", c);
                return;
        }
    }
}
