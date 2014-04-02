#ifndef NAVMESH_H_INCLUDED
#define NAVMESH_H_INCLUDED

#include <stdbool.h>
#include "data_types.h"

void add_to_navmesh(Navmesh* navmesh, int topleft_x, int topleft_y, int bottomright_x, int bottomright_y);

Rect* find_rect(Navmesh *mesh, double x, double y);

void remove_from_navmesh(Navmesh *mesh, Rect *rect);

void connect_rect(Rect *start, Rect *target);

Rect* collides_with_navmesh(Character *character, Navmesh *mesh);

bool is_connected(Rect *rect1, Rect *rect2);

void add_to_linked_list(RectLinkedList *list, Rect *rect);

void destroy_linked_list(RectLinkedList *start);

RectLinkedList* copy_linked_list(RectLinkedList *start, RectLinkedList *output);

double distance(Rect *r1, Rect *r2);

#endif // NAVMESH_H_INCLUDED
