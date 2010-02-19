#ifndef MY_TIME_H
#define MY_TIME_H

#include <time.h>
#include <sys/time.h>

class Time
{
    public:
        static Time now();

        Time();
        Time(long sec, long msec);

        //bool operator<(const Time &) const;
        bool operator>(const Time &) const;

        Time operator+(const Time &) const;

    private:
        timeval tv;
};

inline Time::Time() {}

inline Time::Time(long sec, long msec)
{
    tv.tv_sec = sec;
    tv.tv_usec = msec * 1000;
}

inline Time Time::now()
{
    Time t;
    gettimeofday(&t.tv, NULL);
    return t;
}

inline bool Time::operator>(const Time &rhs) const
{
    long diffSec = tv.tv_sec - rhs.tv.tv_sec;
    if (diffSec == 0) {
        return tv.tv_usec > rhs.tv.tv_usec;
    } else {
        return diffSec > 0;
    }
}

extern Time deadline;

#endif
