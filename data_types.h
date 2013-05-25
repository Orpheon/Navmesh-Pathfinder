#ifndef DATA_TYPES_H_INCLUDED
#define DATA_TYPES_H_INCLUDED

typedef struct
{
    int width;
    int height;
    bool **mask;
} Bitmask;

typedef struct
{
    double x;
    double y;
    double width;
    double height;
} Rect;

typedef struct
{
    int num_rects;
    Rect *list;
} Navmesh;

#endif // DATA_TYPES_H_INCLUDED
