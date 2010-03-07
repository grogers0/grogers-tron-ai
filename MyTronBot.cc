#include "Map.h"
#include "MoveDeciders.h"
#include <vector>
#include <cstdio>
#include <set>
#include <iterator>
#include <algorithm>

#include <sys/time.h>
#include <signal.h>

volatile bool time_expired;

Direction decide_move(const Map &map)
{
    if (isOpponentIsolated(map)) {
        return decideMoveIsolatedFromOpponent(map);
    }

    return decideMoveMinimax(map);
}

void handle_sigalrm(int)
{
    time_expired = true;
}

void send_move(Direction move)
{
    int m = 0;
    switch (move) {
        case NORTH: m = 1; break;
        case EAST: m = 2; break;
        case SOUTH: m = 3; break;
        case WEST: m = 4; break;
    }
    fprintf(stdout, "%d\n", m);
    fflush(stdout);
}



int main()
{
    Map map;

    bool first_time;

    signal(SIGALRM, &handle_sigalrm);

    while (map.readFromFile(stdin))
    {
        time_expired = false;

        itimerval itv;
        itv.it_interval.tv_sec = 0;
        itv.it_interval.tv_usec = 0;

        if (first_time) {
            first_time = false;
            itv.it_value.tv_sec = 2;
            itv.it_value.tv_usec = 900000;
        } else {
            itv.it_value.tv_sec = 0;
            itv.it_value.tv_usec = 950000;
        }
        setitimer(ITIMER_REAL, &itv, NULL);

        send_move(decide_move(map));
    }
    return 0;
}
