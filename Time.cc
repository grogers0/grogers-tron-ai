#include "Time.h"

Time deadline;

Time::Time()
{
}

Time::Time(long sec, long msec)
{
    tv.tv_sec = sec;
    tv.tv_usec = msec * 1000;
}

bool Time::operator<(const Time &rhs) const
{
    long diffSec = tv.tv_sec - rhs.tv.tv_sec;
    if (diffSec == 0) {
        return tv.tv_usec < rhs.tv.tv_usec;
    } else {
        return diffSec < 0;
    }
}
bool Time::operator>(const Time &rhs) const
{
    long diffSec = tv.tv_sec - rhs.tv.tv_sec;
    if (diffSec == 0) {
        return tv.tv_usec > rhs.tv.tv_usec;
    } else {
        return diffSec > 0;
    }
}

Time Time::operator+(const Time &rhs) const
{
    Time ret;

    ret.tv.tv_sec = tv.tv_sec + rhs.tv.tv_sec;
    ret.tv.tv_usec = tv.tv_usec + rhs.tv.tv_usec;
    if (ret.tv.tv_usec >= 1000000) {
        ret.tv.tv_usec -= 1000000;
        ++ret.tv.tv_sec;
    }
    return ret;
}

Time Time::now()
{
    Time t;
    gettimeofday(&t.tv, NULL);
    return t;
}
