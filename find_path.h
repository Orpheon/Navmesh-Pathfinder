#ifndef FIND_PATH_H_INCLUDED
#define FIND_PATH_H_INCLUDED

#include "data_types.h"

RectLinkedList* find_path(Navmesh *mesh, Rect *start, Rect *target);
void find_best_rect(Rect *best_rect, Navmesh *mesh, Rect *target);

#endif // FIND_PATH_H_INCLUDED
