#include "generate_navmesh.h"
#include "data_types.h"
#include "navmesh.h"
#include "simulation.h"
#include <stdlib.h>
#include <math.h>

#ifdef DEBUG_MODE
    #include <stdio.h>
#endif

#define JUMP_HEIGHT 70
#define GRAVITY 0.6
#define STAIR_HEIGHT 6
#define MAX_STAIR_WIDTH 11*6
#define sign(x) ((x>0) - (x<0))

Navmesh* generate_navmesh(Bitmask *map, int char_width, int char_height, double char_speed)
{
    Navmesh *mesh = (Navmesh*) calloc(1, sizeof(Navmesh));
    Rect *rect;
    RectLinkedList *list_iterator;

#ifdef DEBUG_MODE
    printf("\n---GENERATING AREAS---\n");
#endif

    // Generate all the areas, including unnecessary ones
    int x, y, i, j, max_height;
    for (y=0; y<map->height-6; y++)
    {
        x = 0;
        while (x < map->width-6)
        {
            if ((!map->mask[x][y]) && map->mask[x][y+1])
            {
                if (x != 0 && y != 0)
                {
                    i = x;
                    j = y;
                    max_height = -1;
                    while (true)
                    {
                        // Go through all pixels of the possible rectangle to detect collisions
                        j--;

                        if (map->mask[i][j])
                        {
                            // We've hit the wallmask
                            if (max_height == -1)
                            {
                                // This is the first pass, set this to ceiling height and continue
                                max_height = y-j;
                                // Move one row to the right
                                i++;
                                j = y;
                            }
                            else if (max_height > y - j)
                            {
                                // We should have been able to go higher, a new rect has started
                                add_to_navmesh(mesh, x, y, i-x-1, max_height-1);
                                break;
                            }
                        }

                        if (y-j == JUMP_HEIGHT+char_height && max_height != -1)
                        {
                            max_height = JUMP_HEIGHT + char_height;
                        }

                        if (y-j == max_height)
                        {
                            if (map->mask[i][j-1] || y-j == JUMP_HEIGHT+char_height)
                            {
                                i++;
                                j = y;
                                if (!(map->mask[i][j+1]) || map->mask[i][j])
                                {
                                    add_to_navmesh(mesh, x, y, i-x-1, max_height-1);
                                    break;
                                }
                            }
                            else
                            {
                                add_to_navmesh(mesh, x, y, i-x-1, max_height-1);
                                break;
                            }
                        }

                        if (i == map->width)
                        {
                            // max_height will never be -1 here
                            add_to_navmesh(mesh, x, y, i-x, max_height-1);
                        }

                        if (j == 0)
                        {
                            if (max_height == -1)
                            {
                                max_height = y - j;
                                i++;
                                j = y;
                            }
                            else
                            {
                                add_to_navmesh(mesh, x, y, i-x, max_height-1);
                                break;
                            }
                        }
                    }
                    x = i-1;
                }
            }
            x++;
        }
    }

    // Lower them by a set amount, removing all those that are too low
    list_iterator = mesh->list;
    while (list_iterator != 0)
    {
        rect = list_iterator->rect;
        rect->topleft.y += char_height;
        rect->topright.y += char_height;
        list_iterator = list_iterator->next;
        if (rect->topleft.y >= rect->bottomleft.y)
        {
            // This rect was too low for a character to fit through.
            // Destroy the rect
            remove_from_navmesh(mesh, rect);
        }
    }

//    // Stair optimisations
//    // Remove and simplify all rects that are clearly steps of linear stairs
//    printf("\n---OPTIMIZING STAIRS---\n");
//    list_iterator = mesh->list;
//    i = 0;
//    while (list_iterator != 0)
//    {
//        rect = list_iterator->rect;
//        // The rectangle would only be part of a stair if its width is the width of every step
//        int stair_width = rect->bottomright.x - rect->bottomleft.x + 1;
//        if (stair_width >= 1 && stair_width <= MAX_STAIR_WIDTH)
//        {
//            Rect *next_rect = find_rect(mesh, rect->bottomleft.x-stair_width, rect->bottomleft.y+STAIR_HEIGHT);
//            if (next_rect != 0)
//            {
//                // Stair is going left and down
//                while (next_rect != 0)
//                {
//                    // Check whether the stair continues
//                    if (next_rect->bottomright.x - next_rect->bottomleft.x + 1 != stair_width)
//                    {
//                        break;
//                    }
//                    if (rect->topleft.y - rect->bottomleft.y != next_rect->topright.y - next_rect->bottomright.y)
//                    {
//                        break;
//                    }
//
//                    // As long as steps are where they should, merge them in the original one
//                    rect->bottomleft.x = next_rect->bottomleft.x;
//                    rect->bottomleft.y = next_rect->bottomleft.y;
//                    rect->topleft.x = next_rect->topleft.x;
//                    rect->topleft.y = next_rect->topleft.y;
//                    remove_from_navmesh(mesh, next_rect);
//                    next_rect = find_rect(mesh, rect->bottomleft.x-stair_width, rect->bottomleft.y+STAIR_HEIGHT);
//                }
//            }
//            else
//            {
//                // Same as above, only this time for stairs that go right and down
//                x = rect->bottomleft.x;
//                y = rect->bottomleft.y;
//                next_rect = find_rect(mesh, x+stair_width, y+STAIR_HEIGHT);
//                while (next_rect != 0)
//                {
//                    // Check whether the stair continues
//                    if (next_rect->bottomright.x - next_rect->bottomleft.x + 1 != stair_width)
//                    {
//                        break;
//                    }
//                    if (rect->topright.y - rect->bottomright.y != next_rect->topleft.y - next_rect->bottomleft.y)
//                    {
//                        break;
//                    }
//
//                    rect->bottomright.x = next_rect->bottomright.x;
//                    rect->bottomright.y = next_rect->bottomright.y;
//                    rect->topright.x = next_rect->topright.x;
//                    rect->topright.y = next_rect->topright.y;
//                    remove_from_navmesh(mesh, next_rect);
//                    x += stair_width;
//                    y += STAIR_HEIGHT;
//                    next_rect = find_rect(mesh, x+stair_width, y+STAIR_HEIGHT);
//                }
//            }
//        }
//
//        list_iterator = list_iterator->next;
//    }

    // Connect any areas that touch each other
#ifdef DEBUG_MODE
    printf("\n---CONNECTING NEIGHBOURING AREAS---\n");
    fflush(stdout);
#endif
    list_iterator = mesh->list;
    RectLinkedList *other_list_iterator = mesh->list;
    Rect *other_rect;
    int sign1, sign2;
    bool cont = false;
    while (list_iterator != 0)
    {
        rect = list_iterator->rect;
        other_list_iterator = mesh->list;
        while (other_list_iterator != 0)
        {
            other_rect = other_list_iterator->rect;
            // If it's the same rect, no need to even consider connections
            if (rect != other_rect)
            {
                // If it's already connected, ditto
                cont = false;
                for (i=0; i<rect->num_connections; i++)
                {
                    if (rect->connections[i] == other_rect)
                    {
                        cont = true;
                        break;
                    }
                }
                if (!cont)
                {
                    // If other_rect is to the immediate left of rect
                    if (other_rect->bottomright.x == rect->bottomleft.x - 1)
                    {
                        // Check whether the y's connect, which means that the lines from topleft<->bottomright & topright<->bottomleft cross within the rects, ie. they have the same slope signs
                        sign1 = sign(rect->topleft.y - other_rect->bottomright.y);
                        sign2 = sign(other_rect->topright.y - rect->bottomleft.y);

                        if (sign1 == sign2 || sign1 == 0 || sign2 == 0)
                        {
                            connect_rect(rect, other_rect);
                            connect_rect(other_rect, rect);
                        }
                    }
                }

            }
            other_list_iterator = other_list_iterator->next;
        }
        if (rect->num_connections == 0)
        {
            // This rect has no immediate neighbours, which means that jumping should be handled carefully
            rect->is_platform = 1;
        }
        list_iterator = list_iterator->next;
    }

#ifdef DEBUG_MODE
    printf("\n---SIMULATING PLAYER MOVEMENT---\n");
    fflush(stdout);
#endif
    double a, b;
    a = -GRAVITY/2.0;
    b = sqrt(char_speed*char_speed + 8.0*8.0);

    list_iterator = mesh->list;
    while (list_iterator != 0)
    {
        rect = list_iterator->rect;
        test_rectangle(mesh, rect, map, char_width, char_height, char_speed, a, b);
        list_iterator = list_iterator->next;
    }

    return mesh;
}
