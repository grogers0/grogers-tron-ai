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

Map::Map() :
    player_one_x(0),
    player_one_y(0),
    player_two_x(0),
    player_two_y(0),
    map_width(0),
    map_height(0)
{
}

int Map::width() const
{
    return map_width;
}

int Map::height()  const
{
    return map_height;
}

bool Map::isWall(int x, int y) const
{
    if (x < 0 || y < 0 || x >= map_width || y >= map_height) {
        return true;
    } else {
        return is_wall[x][y];
    }
}

bool Map::isWall(Direction dir, Player p) const
{
    int x, y;
    switch (p) {
        case SELF:
            x = player_one_x;
            y = player_one_y;
            break;
        case OPPONENT:
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

void Map::move(Direction dir, Player p)
{
    int *x, *y;
    switch (p) {
        case SELF:
            x = &player_one_x;
            y = &player_one_y;
            break;
        case OPPONENT:
            x = &player_two_x;
            y = &player_two_y;
            break;
    }

    is_wall[*x][*y] = true;

    switch (dir) {
        case NORTH: (*y)--; break;
        case SOUTH: (*y)++; break;
        case WEST: (*x)--; break;
        case EAST: (*x)++; break;
    }
}

int Map::myX() const
{
    return player_one_x;
}

int Map::myY() const
{
    return player_one_y;
}

int Map::opponentX() const
{
    return player_two_x;
}

int Map::opponentY() const
{
    return player_two_y;
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
    is_wall =
        std::vector<std::vector<bool> >(map_width,
                std::vector<bool>(map_height, false));
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
                is_wall[x][y] = true;
                ++x;
                break;
            case ' ':
                if (x >= map_width) {
                    fprintf(stderr, "x >= width in Board_ReadFromStream\n");
                    return;
                }
                is_wall[x][y] = false;
                ++x;
                break;
            case '1':
                if (x >= map_width) {
                    fprintf(stderr, "x >= width in Board_ReadFromStream\n");
                    return;
                }
                is_wall[x][y] = false;
                player_one_x = x;
                player_one_y = y;
                ++x;
                break;
            case '2':
                if (x >= map_width) {
                    fprintf(stderr, "x >= width in Board_ReadFromStream\n");
                    return;
                }
                is_wall[x][y] = false;
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
