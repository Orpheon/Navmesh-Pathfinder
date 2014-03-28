#include "find_path.h"
#include "data_types.h"
#include "navmesh.h"
#include <math.h>

#define INACTIVE 0
#define ACTIVE 1
#define VISITED 2

RectLinkedList *find_path(Navmesh *mesh, Rect *start, Rect *target)
{
    RectLinkedList *list_iterator, *path;
    Rect *rect, *other_rect;

    start->distance = 0.0;
    while (true)
    {
        Rect *rect = find_best_rect(mesh, target);
        if (rect != 0)
        {
            add_to_linked_list(rect->history, rect);
            for (int i=0; i<rect->num_connections; i++)
            {
                other_rect = rect->connections[i];
                if (other_rect.activation == INACTIVE || rect->distance + distance(rect, other_rect))
                {
                    other_rect->distance = rect->distance + distance(rect, other_rect);
                    other_rect->history = copy_linked_list(rect->history);
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

        path = target->history;
    }

    // Get rid of all lingering data for the next run
    list_iterator = mesh->list;
    while (list_iterator != 0)
    {
        rect = list_iterator->rect;
        rect->distance = -1.0;
        rect->activation = INACTIVE;
        destroy_linked_list(rect->history);
        list_iterator = list_iterator->next;
    }

    return path;
}

Rect* find_best_rect(Navmesh *mesh, Rect *target)
{
    RectLinkedList *list_iterator;
    Rect *rect, *best_rect=0;
    int fitness = MAX_INT;

    list_iterator = mesh->list;
    while (list_iterator != 0)
    {
        rect = list_iterator->rect;
        if (rect.activation == ACTIVE)
        {
            if (rect.distance + distance(rect, target) < fitness)
            {
                fitness = rect.distance + distance(rect, target);
                best_rect = rect;
            }
        }

        list_iterator = list_iterator->next;
    }
}
