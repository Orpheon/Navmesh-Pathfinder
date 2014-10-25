#ifndef SIMULATION_H_INCLUDED
#define SIMULATION_H_INCLUDED

#include <stdbool.h>
#include "data_types.h"

void test_rectangle(Navmesh *mesh, Rect *rect, Bitmask *map, int char_width, int char_height, double char_speed, double jump_a, double jump_b);
bool collides_with_wallmask(Character *character, Bitmask *map);
bool is_floor_underneath(Character *character, Bitmask *map, int depth);

#endif // SIMULATION_H_INCLUDED
