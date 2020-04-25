#ifndef MOVE_H_INCLUDED
#define MOVE_H_INCLUDED

#include "data_types.h"

char* get_commands(Character *character, Rect *current_rect, Rect *next_rect);
int sign(int x);

#endif // MOVE_H_INCLUDED
