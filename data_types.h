#ifndef DATA_TYPES_H_INCLUDED
#define DATA_TYPES_H_INCLUDED

struct Bitmask
{
    int width;
    int height;
    bool **mask;
};

struct Point
{
    int x;
    int y;
};

struct Rect
{
    Point topleft;
    Point topright;
    Point bottomleft;
    Point bottomright;
    Rect *connections;
    int num_connections;
};

struct RectLinkedList
{
    RectLinkedList *next;
    Rect *rect;
};

struct Navmesh
{
    int num_rects;
    RectLinkedList *list;
};

#endif // DATA_TYPES_H_INCLUDED
