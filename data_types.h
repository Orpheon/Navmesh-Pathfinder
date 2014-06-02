#ifndef DATA_TYPES_H_INCLUDED
#define DATA_TYPES_H_INCLUDED

#include <stdbool.h>

typedef struct Bitmask Bitmask;
typedef struct Point Point;
typedef struct Rect Rect;
typedef struct RectLinkedList RectLinkedList;
typedef struct Navmesh Navmesh;
typedef struct Character Character;

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
    bool is_platform;
    Rect **connections;
    int num_connections;
    // Pathfinding stuff
    RectLinkedList *history;
    double distance;
    int activation;
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

struct Character
{
    double x;
    double y;
    double hs;
    double vs;
    double width;
    double height;
    double speed;
};

#endif // DATA_TYPES_H_INCLUDED
