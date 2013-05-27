#ifndef GENERATE_NAVMESH_H_INCLUDED
#define GENERATE_NAVMESH_H_INCLUDED

Navmesh* generate_navmesh(Bitmask *map, int char_width, int char_height, double char_speed);

void add_to_navmesh(Navmesh* navmesh, int topleft_x, int topleft_y, int bottomright_x, int bottomright_y);

Rect* find_rect(Navmesh *mesh, double x, double y);

void remove_from_navmesh(Navmesh *mesh, Rect *rect);

#endif // GENERATE_NAVMESH_H_INCLUDED
