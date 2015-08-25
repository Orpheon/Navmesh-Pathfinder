#include "move.h"
#include "data_types.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define GRAVITY 0.6

#define OUTPUT_LENGTH 2
#define DIRECTION 0
#define STANDSTILL 0
#define DIR_LEFT -1
#define DIR_RIGHT 1
#define JUMP 1

char* get_commands(Character *character, Rect *current_rect, Rect *next_rect)
{
    char *output = calloc(sizeof(char), OUTPUT_LENGTH);

    double hs_left, hs_right;
    hs_left = fmin(character->hs-6.0, -6.0);
    hs_right = fmin(character->hs+6.0, 6.0);

#ifdef DEBUG_MODE
    printf("\n\nINPUT GATHERING\n\n");
#endif

    // If we're not inside our current/last rectangle, then we are in the air flying towards our destination, so we should just continue to do that
    if (character->x < current_rect->bottomleft.x || character->x > current_rect->bottomright.x || character->y < current_rect->topleft.y || character->y > current_rect->bottomleft.y)
    {
        // Get the equation of the parabola of our current falling (in the form y = ax^2 + bx + c)
        double a, b, c;
        a = GRAVITY/2;
        b = -sqrt(character->hs*character->hs + character->vs*character->vs) - 2*a*character->x;
        c = character->y - a*character->x*character->x - b*character->x;

        // Test whether we'll land within the right boundaries with the current path
        double landing_x;
        if (b*b - 4*a*(c-next_rect->bottomleft.y) > 0)
        {
            landing_x = (-b + sqrt(b*b - 4*a*(c-next_rect->bottomleft.y)))/(2*a);
        }
        else
        {
            // We're off track anyways, just continue walking and let things take care of themselves
            landing_x = 0.5*(next_rect->bottomleft.x + next_rect->bottomright.x);
        }

        if (landing_x < next_rect->bottomleft.x)
        {
            // We're overshooting on the left side
            output[DIRECTION] = DIR_RIGHT;
        }
        else if (landing_x > next_rect->bottomright.x)
        {
            // We're overshooting on the right side
            output[DIRECTION] = DIR_LEFT;
        }
        else
        {
            // We're good, but we might get to where-ever faster if we continued moving in the direction we were going until now
            if (current_rect->bottomleft.x > next_rect->bottomleft.x)
            {
                output[DIRECTION] = DIR_LEFT;
            }
            else if (current_rect->bottomleft.x < next_rect->bottomleft.x)
            {
                output[DIRECTION] = DIR_RIGHT;
            }
            else
            {
                output[DIRECTION] = STANDSTILL;
            }
        }

        // It does not make any sense for us to jump right now, assuming no doublejump
        output[JUMP] = 0;
        return output;
    }

    // Target rect is underneath us
    if (sign(current_rect->bottomright.x - next_rect->bottomright.x) != sign(current_rect->bottomleft.x - next_rect->bottomleft.x))
    {
        // We need to find what side to walk/jump off of
        if (abs(current_rect->bottomleft.x - next_rect->bottomleft.x) < abs(current_rect->bottomright.x - next_rect->bottomright.x))
        {
            output[DIRECTION] = DIR_LEFT;
        }
        else
        {
            output[DIRECTION] = DIR_RIGHT;
        }
        // There is no occasion where we'd want to jump, so don't do it
        output[JUMP] = 0;
        return output;
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
            b = -sqrt(hs_right*hs_right + character->vs*character->vs) - 2*a*current_rect->bottomright.x;
            c = current_rect->bottomright.y - a*current_rect->bottomright.x*current_rect->bottomright.x - b*current_rect->bottomright.x;

            double landing_x, determinant;
            // First check whether "walking off" even makes sense (it doesn't if we need to go higher)
            if (next_rect->bottomleft.y > current_rect->bottomright.y)
            {
                determinant = b*b - 4*a*(c-next_rect->bottomleft.y);
                if (determinant >= 0)
                {
                    // A sqrt is always positive, and we want the larger one of the two solutions (the one on the right)
                    landing_x = (-b + sqrt(determinant))/(2*a);
                    if ((landing_x >= next_rect->bottomleft.x && landing_x <= next_rect->bottomright.x) || !next_rect->is_platform)
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
            }

            // Would jumping help?
            // Calculate the jumping parabola from our current position
            double v_y = fmin(character->vs - 8, 8);
            b = -sqrt(hs_right*hs_right + v_y*v_y) - 2*a*character->x;
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
        // If target rect is low enough to just be able to walk over (flat or stair, both up or down) and near enough
        if (abs(current_rect->bottomright.y - next_rect->bottomleft.y) <= 6 && current_rect->bottomleft.x-6 == next_rect->bottomright.x)
        {
            // Just go on walking
            output[DIRECTION] = DIR_LEFT;
            output[JUMP] = 0;
            return output;
        }
        // We might need to jump
        else
        {
            // Get the equation of the parabola of walking off the edge (in the form y = ax^2 + bx + c)
            double a, b, c;
            a = GRAVITY/2;
            b = -sqrt(hs_left*hs_left + character->vs*character->vs) - 2*a*current_rect->bottomleft.x;
            c = current_rect->bottomleft.y - a*current_rect->bottomleft.x*current_rect->bottomleft.x - b*current_rect->bottomleft.x;

            double landing_x, determinant;
            // First check whether "walking off" even makes sense (it doesn't if we need to go higher)
            if (next_rect->bottomright.y > current_rect->bottomleft.y)
            {
                determinant = b*b - 4*a*(c-next_rect->bottomleft.y);
                if (determinant >= 0)
                {
                    // A sqrt is always positive, and we want the smaller one of the two solutions (the one on the left)
                    landing_x = (-b - sqrt(determinant))/(2*a);
                    if ((landing_x >= next_rect->bottomleft.x && landing_x <= next_rect->bottomright.x) || !next_rect->is_platform)
                    {
                        // Just walking off at current speeds will grant us safe landing.
                        // Do so.
                        output[DIRECTION] = DIR_LEFT;
                        output[JUMP] = 0;
                        return output;
                    }

                    // Would we be going too far?
                    if (landing_x < next_rect->bottomleft.x)
                    {
                        // Should we already be braking now?
                        c = character->y - a*character->x*character->x - b*character->x;
                        determinant = b*b - 4*a*(c-next_rect->bottomleft.y);
                        if (determinant >= 0)
                        {
                            // Check whether walking off from our -current- position would already be enough to overshoot
                            landing_x = (-b + sqrt(determinant))/(2*a);
                            if (landing_x < next_rect->bottomleft.x)
                            {
                                // It's high time we brake
                                output[DIRECTION] = DIR_LEFT;
                                output[JUMP] = 0;
                                return output;
                            }
                        }
                    }
                }
            }

            // Would jumping help?
            // Calculate the jumping parabola from our current position
            double v_y = fmin(character->vs - 8, 8);
            b = sqrt(hs_left*hs_left + v_y*v_y) - 2*a*character->x;
            c = character->y - a*character->x*character->x - b*character->x;
            // It is not possible for this sqrt to be invalid. If this were the case, the navmesh has to be broken
            landing_x = (-b - sqrt(b*b - 4*a*(c-next_rect->bottomleft.y)))/(2*a);
            if (landing_x >= next_rect->bottomleft.x && landing_x <= next_rect->bottomright.x)
            {
                // Jumping would work right now
                output[DIRECTION] = DIR_LEFT;
                output[JUMP] = 1;
                return output;
            }

            // Nothing to do right now, lets just keep running in the right direction until something new happens
            output[DIRECTION] = DIR_LEFT;
            output[JUMP] = 0;
            return output;
        }
    }
    // Should never happen
    else
    {
#ifdef DEBUG_MODE
        printf("\n\nERROR: Control end in move.c - This requires investigation.");
        fflush(stdout);
#endif
        output[DIRECTION] = DIR_LEFT;
        output[JUMP] = 1;
        return output;
    }
}

int sign(int x)
{
    return (x > 0) - (x < 0);
}
