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
#define sign(x) ((x > 0) - (x < 0))

void test_rectangle(Navmesh *mesh, Rect *rect, Bitmask *map, int char_width, int char_height, double char_speed, double jump_a, double jump_b)
{
    Character *character = (Character*) calloc(1, sizeof(Character));

    character->width = char_width;
    character->height = char_height;
    character->speed = char_speed;

    // DEBUGTOOL
    int debug_flag = 0;
    if (point_inside_rect(mesh, 3433, 220) == rect)
    {
        printf("\n\n\nDebug Rect coming up");
        printf("\nTopleft: %i %i", rect->topleft.x, rect->topleft.y);
        printf("\nBottomright: %i %i", rect->bottomright.x, rect->bottomright.y);
        debug_flag = 1;
        fflush(stdout);
    }
    else if (debug_flag)
    {
        debug_flag = 0;
    }

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

                double rect_width, speed;
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
                        if (debug_flag)
                        {
                            printf("\n\nJumping from top:");
                        }
                        character->y -= rect->bottomleft.y - rect->topleft.y;
                        character->vs = 0;
//                            // First, we need to deal with rectangles that are too narrow to properly jump in
//                            rect_width = rect->bottomright.x - rect->bottomleft.x;
//                            // If the rect is wide enough to get to the apex of a jump
//                            if (rect_width >= -jump_b/(2.0*jump_a))
//                            {
//                                // We don't need to care about this
//                                character->y -= rect->bottomleft.y - rect->topleft.y;
//                                character->vs = 0;
//                            }
//                            else
//                            {
//                                // This rect is very narrow
//                                // Calculate the height we will be exiting at if starting from earliest point, as well as vertical speed
//                                character->y -= jump_a*rect_width*rect_width + jump_b*rect_width;
//                                speed = 2*jump_a*rect_width + jump_b;
//                                character->vs = sqrt(speed*speed - character->speed*character->speed);
//                            }
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
//                    if (debug_flag && direction == 6 && counter == 1)
//                    {
//                        printf("\n\nX: %f\nY: %f\nHS: %f\nVS: %f", character->x, character->y, character->hs, character->vs);
//                    }

                    if (debug_flag && direction == 6 && counter == 1)
                    {
                        printf("\n\nX: %f\nY: %f\nHS: %f\nVS: %f", character->x+character->width, character->y, character->hs, character->vs);
                    }

                    // Apply gravity
                    character->vs += GRAVITY;
                    character->vs = min(character->vs, 10);

                    // Move carefully, while checking to collisions
                    // Don't want to do real collision handling
                    for (int i=0; i<SIMULATION_GRANULARITY; i++)
                    {
//                        if (debug_flag && character->x >= 836.0 && character->y >= 1471.0)
//                        {
//                            printf("\nIn simulation: %f %f", character->x, character->y);
//                        }

                        character->x += character->hs/SIMULATION_GRANULARITY;
                        character->y += character->vs/SIMULATION_GRANULARITY;

                        // Check whether we've landed in an area
                        Rect *new_rect = collides_with_navmesh(character, mesh);
                        if (new_rect != 0)
                        {
                            // Check whether we're on the ground
                            if (character->vs >= 0 && is_floor_underneath(character, map, character->vs))
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
                            if (debug_flag && direction == -6 && counter == 2)
                            {
                                printf("\n\nCollision\nX: %f\nY: %f\nHS: %f\nVS: %f\n", character->x+character->width, character->y, character->hs, character->vs);
                                printf("\n%i", collides_with_wallmask(character, map));
                            }
                            // Abuse of triangle inequality, this is very hacky math
                            // Only adding character->hs runs the risk of /just/ not getting out
                            // Since the sum of both components is necessarily larger than the diagonal, it avoids this problem while still having a rather low bound
                            character->x -= sign(character->hs)*(fabs(character->hs)+fabs(character->vs))/SIMULATION_GRANULARITY;
                            collision_counter++;
                            if (collision_counter > SIMULATION_GRANULARITY)
                            {
                                // we got stuck somewhere, give up
                                printf("\n\nCollision got stuck\nX: %f\nY: %f\nHS: %f\nVS: %f\n", character->x+character->width, character->y, character->hs, character->vs);
                                printf("\n%i", collides_with_wallmask(character, map));
                                printf("\nDirection: %i", direction);
                                printf("\nCounter: %i", counter);
                                printf("\nRect bottomleft: %i | %i", rect->bottomleft.x, rect->bottomleft.y);
                                printf("\nRect bottomright: %i | %i", rect->bottomright.x, rect->bottomright.y);
                                printf("\nRect topleft: %i | %i", rect->topleft.x, rect->topleft.y);
                                printf("\nRect topright: %i | %i", rect->topright.x, rect->topright.y);
                                printf("\nRect pointer: %d", rect);
                                exit(1);

                                continue_loop = false;
                                break;
                            }
                        }

                        if (debug_flag)
                        {
                            printf("\n\nPost-Collision:\nX: %f\nY: %f\nHS: %f\nVS: %f", character->x+character->width, character->y, character->hs, character->vs);
                            printf("\nValues: %i, %i", collides_with_navmesh(character, mesh), collides_with_wallmask(character, map));
                        }

                        if (!continue_loop)
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
}


bool collides_with_wallmask(Character *character, Bitmask *map)
{
    for (int w=0; w<character->width; w++)
    {
        for (int h=0; h<character->height; h++)
        {
            // Check for every pixel in character, is there wallmask there?
            if (map->mask[(int)character->x + w][(int)character->y - h])
            {
                return true;
            }
        }
    }

    return false;
}


bool is_floor_underneath(Character *character, Bitmask *map, int depth)
{
    for (int h=0; h<depth; h++)
    {
        for (int w=0; w<character->width; w++)
        {
            // Check for every pixel in character, is there wallmask there?
            if (map->mask[(int)character->x + w][(int)character->y - h])
            {
                return true;
            }
        }
    }

    return false;
}
