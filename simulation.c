#include <stdbool.h>
#include "simulation.h"
#include "data_types.h"
#include "navmesh.h"
#include <stdlib.h>

// DEBUGTOOL
#include <stdio.h>

#define SIMULATION_GRANULARITY 10
#define GRAVITY 0.6
#define min(x, y) (x<=y ? x : y)

void test_rectangle(Navmesh *mesh, Rect *rect, Bitmask *map, int char_width, int char_height, double char_speed)
{
    Character *character = (Character*) calloc(1, sizeof(Character));

    character->width = char_width;
    character->height = char_height;
    character->speed = char_speed;

    // DEBUGTOOL
    int debug_flag = 0;
//    if (point_inside_rect(mesh, 4161, 548) == rect)
//    {
//        printf("\n\n\nDebug Rect coming up");
//        printf("\nTopleft: %i %i", rect->topleft.x, rect->topleft.y);
//        printf("\nBottomright: %i %i", rect->bottomright.x, rect->bottomright.y);
//        debug_flag = 1;
//        fflush(stdout);
//    }

    // Simulate going in either direction (+6 and -6)
    for (int direction = -6; direction < 7; direction += 12)
    {
        // Try out all 3 different movement types (walking off, jumping off and flying off from the apex of a jump) on this side
        for (int counter=0; counter<3; counter++)
        {
            for (int input_direction = -1; input_direction < 2; input_direction++)
            {
                // Reset the character at the correct position on the edge of the mask
                if (direction == 6)
                {
                    character->x = (double) rect->bottomright.x;
                    character->y = (double) rect->bottomright.y;
                }
                else
                {
                    character->x = (double) rect->bottomleft.x - (double) character->width;
                    character->y = (double) rect->bottomleft.y;
                }

                if (debug_flag)
                {
                    printf("\n\nDirection: %i\nX: %f\nY: %f", direction, character->x, character->y);
                }

                switch (counter)
                {
                    case 0:
                        character->vs = 0;
                        if (debug_flag)
                        {
                            printf("\n\nWalking off:");
                        }
                        break;

                    case 1:
                        character->vs = -8;
                        if (debug_flag)
                        {
                            printf("\n\nJumping off:");
                        }
                        break;

                    case 2:
                        character->vs = 0;
                        character->y -= (rect->bottomleft.y - rect->topleft.y);
                        if (debug_flag)
                        {
                            printf("\n\nJumping from:");
                        }
                        break;

                    default:
                        printf("\nERROR: Invalid counter value in simulation.c");
                }

                if (debug_flag)
                {
                    printf("\n\nInput: %i\nX: %f\nY: %f", input_direction, character->x, character->y);
                }
                // Try flying left and then right
//                character->x += direction;
                character->hs = (double)input_direction * character->speed;

                if (collides_with_wallmask(character, map))
                {
                    // If we're already in a collision before moving, no sense trying to do anything
                    if (debug_flag)
                    {
                        printf("\n\nCOLLISION BEFORE SIMULATION");
                    }
                    break;
                }

                bool continue_loop = true;
                while (continue_loop)
                {
                    if (debug_flag)
                    {
                        printf("\n\nX: %f\nY: %f\nHS: %f\nVS: %f", character->x, character->y, character->hs, character->vs);
                    }

                    // Apply gravity
                    character->vs += GRAVITY;
                    character->vs = min(character->vs, 10);

                    // Move carefully, while checking to collisions
                    // Don't want to do real collision handling
                    for (int i=0; i<SIMULATION_GRANULARITY; i++)
                    {
                        character->x += character->hs/SIMULATION_GRANULARITY;
                        character->y += character->vs/SIMULATION_GRANULARITY;

                        // Check whether we've landed in an area
                        Rect *new_rect = collides_with_navmesh(character, mesh);
                        if (new_rect != 0)
                        {
                            // Check whether we've landed or are flying up
                            if (character->vs > 0)
                            {
                                // Are we already connected?
                                if (new_rect != rect)
                                {
                                    if (!is_connected(rect, new_rect))
                                    {
                                        connect_rect(rect, new_rect);
                                    }
                                }
                                // If yes, just continue
                                continue_loop = false;
                                break;
                            }
                        }

                        // Respond to collisions with wallmask
                        // We can't collide vertically with anything as long as we check for areas first
                        int collision_counter = 0;
                        while (collides_with_wallmask(character, map))
                        {
                            character->x -= character->hs/SIMULATION_GRANULARITY;
                            collision_counter++;
                            if (collision_counter > SIMULATION_GRANULARITY)
                            {
                                // we got stuck somewhere, give up
                                continue_loop = false;
                                break;
                            }
                        }

                        if (!continue_loop)
                        {
                            break;
                        }
                    }
                }
            }
        }
        debug_flag = 0;
    }
}


bool collides_with_wallmask(Character *character, Bitmask *map)
{
    bool collided = false;

    for (int w=0; w<character->width; w++)
    {
        for (int h=0; h<character->height; h++)
        {
//            printf("\nRound:\n%i, %i", (int)character->x + w, (int)character->y - h);
//            printf("\n%i, %i", w, h);
//            printf("\n%i, %i\n", map->width, map->height);
//            fflush(stdout);
            // Check for every pixel in character, is there wallmask there?
            if (map->mask[(int)character->x + w][(int)character->y - h])
            {
                collided = true;
                break;
            }
        }
        if (collided)
        {
            break;
        }
    }

    return collided;
}
