#ifndef POSITION_H
#define POSITION_H

struct position
{
    int x, y;

    position();
    position(int x, int y);

    position north() const;
    position south() const;
    position east() const;
    position west() const;
};

inline
position::position()
{
}

inline
position::position(int x, int y) :
    x(x), y(y)
{
}

inline
position position::north() const
{
    return position(x, y - 1);
}

inline
position position::south() const
{
    return position(x, y + 1);
}

inline
position position::east() const
{
    return position(x + 1, y);
}

inline
position position::west() const
{
    return position(x - 1, y);
}


inline
bool operator==(position pos1, position pos2)
{
    return pos1.x == pos2.x && pos1.y == pos2.y;
}

inline
bool operator!=(position pos1, position pos2)
{
    return !(pos1 == pos2);
}

inline
bool operator<(position pos1, position pos2)
{
    if (pos1.x == pos2.x)
        return pos1.y < pos2.y;
    return pos1.x < pos2.x;
}

#endif
