#include "move.h"
#include "data_types.h"

#include <math.h>

#define GRAVITY 0.6

#define OUTPUT_LENGTH 2
#define DIRECTION 0
#define DIR_LEFT -1
#define DIR_RIGHT 1
#define JUMP 1

char* get_commands(Navmesh *mesh, Character *character, Rect *current_rect, Rect *next_rect)
{
    // This needs some serious thought as to best way to implement this. In principle every step is easy, but checking without replicating too much isn't.

    char[OUTPUT_LENGTH] output;

    // Target rect is underneath us
    if (sign(current_rect->bottomright.x - next_rect->bottomright.x) != sign(current_rect->bottomleft.x - next_rect->bottomleft.x))
    {
        // TODO
    }
    // Target rect is on our right
    else if (current_rect->bottomright.x < next_rect->bottomleft.x)
    {
        // If target rect is low enough to just be able to walk over (flat or stair, both up or down) and near enough
        if (abs(current_rect->bottomright.y - next_rect->bottomleft.y) <= 6 && current_rect->bottomright.x+6 == next_rect->bottomleft.x)
        {
            // Just go on walking
            output[DIRECTION] = DIR_RIGHT;
            output[JUMP] = 0;
            return output;
        }
        // We might need to jump
        else
        {
            // Get the equation of the parabola of walking off the edge (in the form y = ax^2 + bx + c)
            double a, b, c;
            a = GRAVITY/2;
            b = sqrt(character->hs*character->hs + character->vs*character->vs) - 2*a*current_rect->bottomright.x;
            c = current_rect->bottomright.y - a*current_rect->bottomright.x*current_rect->bottomright.x - b*current_rect->bottomright.x;

            double landing_x, determinant;
            // First check whether "walking off" even makes sense (it doesn't if we need to go higher)
            determinant = b*b - 4*a*(c-next_rect->bottomleft.y);
            if (determinant >= 0)
            {
                // A sqrt is always positive, and we want the larger one of the two solutions (the one on the right)
                landing_x = (-b + sqrt(determinant))/(2*a);
                if (landing_x >= next_rect->bottomleft.x && landing_x <= next_rect->bottomright.x)
                {
                    // Just walking off at current speeds will grant us safe landing.
                    // Do so.
                    output[DIRECTION] = DIR_RIGHT;
                    output[JUMP] = 0;
                    return output;
                }

                // Would we be going too far?
                if (landing_x > next_rect->bottomright.x)
                {
                    // Should we already be braking now?
                    c = character->y - a*character->x*character->x - b*character->x;
                    determinant = b*b - 4*a*(c-next_rect->bottomleft.y);
                    if (determinant >= 0)
                    {
                        // Check whether walking off from our -current- position would already be enough to overshoot
                        landing_x = (-b + sqrt(determinant))/(2*a);
                        if (landing_x > next_rect->bottomright.x)
                        {
                            // It's high time we brake
                            output[DIRECTION] = DIR_LEFT;
                            output[JUMP] = 0;
                            return output;
                        }
                    }
                }
            }

            // Would jumping help?
            // Calculate the jumping parabola from our current position
            double v_y = character->vs - 8;
            b = sqrt(character->hs*character->hs + v_y*v_y) - 2*a*character->x;
            c = character->y - a*character->x*character->x - b*character->x;
            // It is not possible for this sqrt to be invalid. If this were the case, the navmesh has to be broken
            landing_x = (-b + sqrt(b*b - 4*a*(c-next_rect->bottomleft.y)))/(2*a);
            if (landing_x >= next_rect->bottomleft.x && landing_x <= next_rect->bottomright.x)
            {
                // Jumping would work right now
                output[DIRECTION] = DIR_RIGHT;
                output[JUMP] = 1;
                return output;
            }

            // Nothing to do right now, lets just keep running in the right direction until something new happens
            output[DIRECTION] = DIR_RIGHT;
            output[JUMP] = 0;
            return output;
        }
    }
    // Target rect is on our left
    else if (current_rect->bottomleft.x > next_rect->bottomright.x)
    {

    }
}

int sign(int x)
{
    return (x > 0) - (x < 0);
}
