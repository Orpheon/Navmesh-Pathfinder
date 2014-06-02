#include "find_path.h"
#include "data_types.h"
#include "navmesh.h"
#include <math.h>
#include <limits.h>
// DEBUGTOOL
#include <stdio.h>

#define INACTIVE 0
#define ACTIVE 1
#define VISITED 2

RectLinkedList* find_path(Navmesh *mesh, Rect *start, Rect *target)
{
    RectLinkedList *list_iterator;
    Rect *rect=0, *other_rect=0;

    // DEBUGTOOL
    int debug_flag = 0;
    if (debug_flag)
    {
        printf("\n\n\nBEGIN PATHFINDING DEBUG\n");
    }

    start->distance = 0.0;
    start->activation = ACTIVE;
    while (true)
    {
        rect = find_best_rect(mesh, target);
        if (debug_flag)
        {
            printf("\n\nNew rect observed; next rect=%d", rect);
        }
        if (rect != 0)
        {
            rect->history = add_to_linked_list(rect->history, rect);
            for (int i=0; i<rect->num_connections; i++)
            {
                other_rect = rect->connections[i];
                if (other_rect->activation == INACTIVE || rect->distance + distance(rect, other_rect) < other_rect->distance)
                {
                    if (debug_flag && other_rect == target)
                    {
                        printf("\n\nTarget (%i|%i) got updated", other_rect->bottomleft.x, other_rect->bottomleft.y);
                    }
                    // Overwrite everything
                    other_rect->distance = rect->distance + distance(rect, other_rect);
                    destroy_linked_list(other_rect->history, 0);
                    other_rect->history = copy_linked_list(rect->history);
                    if (debug_flag && other_rect == target)
                    {
                        RectLinkedList *l = other_rect->history;
                        printf("\n\nResulting path:\n");
                        while (l != 0)
                        {
                            printf("(%i|%i)   ", l->rect->bottomleft.x, l->rect->bottomleft.y);
                            l = l->next;
                        }
                        printf("\nPath done\n");
                    }
                    other_rect->activation = ACTIVE;
                    if (target->activation != INACTIVE)
                    {
                        if (target->distance < other_rect->distance)
                        {
                            // No sense in continuing this path anymore
                            other_rect->activation = VISITED;
                        }
                    }
                }
            }
            rect->activation = VISITED;
        }
        else
        {
            // No active nodes left
            // We're done
            break;
        }
    }

    RectLinkedList *path = copy_linked_list(target->history);

    // Get rid of all lingering data for the next run
    list_iterator = mesh->list;
    while (list_iterator != 0)
    {
        rect = list_iterator->rect;
        rect->distance = -1.0;
        rect->activation = INACTIVE;
        destroy_linked_list(rect->history, 0);
        rect->history = 0;
        list_iterator = list_iterator->next;
    }

    return path;
}

Rect* find_best_rect(Navmesh *mesh, Rect *target)
{
    RectLinkedList *list_iterator;
    Rect *rect, *best_rect=0;
    int fitness = INT_MAX;

    list_iterator = mesh->list;
    while (list_iterator != 0)
    {
        rect = list_iterator->rect;
        if (rect->activation == ACTIVE)
        {
            if (rect->distance + distance(rect, target) < fitness)
            {
                fitness = rect->distance + distance(rect, target);
                best_rect = rect;
            }
        }

        list_iterator = list_iterator->next;
    }
    return best_rect;
}
