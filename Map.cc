// Map.cc

#include "Map.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

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

void Map::sendMoveToServer(Direction move)
{
    fprintf(stdout, "%d\n", move);
    fflush(stdout);
}

bool Map::readFromFile(FILE *file_handle)
{
    int c;
    int num_items = fscanf(file_handle, "%d %d\n", &width, &height);
    if (feof(file_handle) || num_items < 2) {
        return false;
    }
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
                ++pos.x;
                break;
            case '2':
                if (pos.x >= width) {
                    fprintf(stderr, "x >= width in Board_ReadFromStream\n");
                    return false;
                }
                is_wall[index(pos)] = true;
                player_pos[1] = pos;
                ++pos.x;
                break;
            default:
                fprintf(stderr, "unexpected character %d in Board_ReadFromStream", c);
                return false;
        }
    }

    return true;
}
