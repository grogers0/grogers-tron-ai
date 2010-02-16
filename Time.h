#ifndef MY_TIME_H
#define MY_TIME_H

#include <time.h>

class Time
{
    public:
        static Time now();

        Time();
        Time(long sec, long msec);

        bool operator<(const Time &) const;
        bool operator>(const Time &) const;

        Time operator+(const Time &) const;

    private:
        timespec ts;
};


extern Time deadline;

#endif
