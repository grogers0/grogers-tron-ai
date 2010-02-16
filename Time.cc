#include "Time.h"

Time deadline;

Time::Time()
{
}

Time::Time(long sec, long msec)
{
    ts.tv_sec = sec;
    ts.tv_nsec = msec * 1000000l;
}

bool Time::operator<(const Time &rhs) const
{
    long diffSec = ts.tv_sec - rhs.ts.tv_sec;
    if (diffSec == 0) {
        return ts.tv_nsec < rhs.ts.tv_nsec;
    } else {
        return diffSec < 0;
    }
}
bool Time::operator>(const Time &rhs) const
{
    long diffSec = ts.tv_sec - rhs.ts.tv_sec;
    if (diffSec == 0) {
        return ts.tv_nsec > rhs.ts.tv_nsec;
    } else {
        return diffSec > 0;
    }
}

Time Time::operator+(const Time &rhs) const
{
    Time ret;

    ret.ts.tv_sec = ts.tv_sec + rhs.ts.tv_sec;
    ret.ts.tv_nsec = ts.tv_nsec + rhs.ts.tv_nsec;
    if (ret.ts.tv_nsec >= 1000000000l) {
        ret.ts.tv_nsec -= 1000000000l;
        ++ret.ts.tv_sec;
    }
    return ret;
}

Time Time::now()
{
    Time t;
    clock_gettime(CLOCK_MONOTONIC, &t.ts);
    return t;
}
