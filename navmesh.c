#include <stdbool.h>
#include "data_types.h"
#include "navmesh.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
// DEBUGTOOL
#include <stdio.h>

void add_to_navmesh(Navmesh* mesh, int bottomleft_x, int bottomleft_y, int width, int height)
{
    Rect *r = (Rect*) calloc(1, sizeof(Rect));

//    printf("\n\nPosition: (%i | %i), (%i | %i)", bottomleft_x, bottomleft_y, bottomleft_x+width, bottomleft_y+height);

    // Positions
    r->topleft.x = bottomleft_x;
    r->topleft.y = bottomleft_y-height;
    r->bottomleft.x = bottomleft_x;
    r->bottomleft.y = bottomleft_y;
    r->topright.x = bottomleft_x + width;
    r->topright.y = bottomleft_y - height;
    r->bottomright.x = bottomleft_x + width;
    r->bottomright.y = bottomleft_y;

    // Add it to the list
    RectLinkedList *link = (RectLinkedList*) calloc(1, sizeof(RectLinkedList));
    link->rect = r;
    link->next = 0;

    RectLinkedList *tmp = mesh->list;
    if (tmp == 0)
    {
        mesh->list = link;
    }
    else
    {
        while (tmp->next != 0)
        {
            tmp = tmp->next;
        }
        tmp->next = link;
    }

    mesh->num_rects++;
}

Rect* find_rect(Navmesh *mesh, double x, double y)
{
    // Returns a rect if there's one with the bottomleft corner at that position, 0 if there isn't
    RectLinkedList *l = mesh->list;
    Rect *r;

    while (l != 0)
    {
        r = l->rect;
        if (r->bottomleft.x == x && r->bottomleft.y == y)
        {
            return r;
        }
        l = l->next;
    }
    return 0;
}

void remove_from_navmesh(Navmesh *mesh, Rect *rect)
{
    RectLinkedList *l = mesh->list;
    if (l->rect == rect)
    {
        mesh->list = l->next;
        mesh->num_rects--;
        free(rect);
        free(l);
    }
    while (l->next != 0)
    {
        if (l->next->rect == rect)
        {
            RectLinkedList *tmp;
            tmp = l->next->next;
            free(rect);
            free(l->next);
            l->next = tmp;
            mesh->num_rects--;
        }
        l = l->next;
    }
}

void connect_rect(Rect *start, Rect *target)
{
    Rect **tmp;
    tmp = (Rect**) calloc(start->num_connections+1, sizeof(Rect*));
    memcpy(tmp, start->connections, sizeof(Rect*)*start->num_connections);
    tmp[start->num_connections] = target;
    free(start->connections);
    start->connections = tmp;
    start->num_connections++;
}

Rect* collides_with_navmesh(Character *character, Navmesh *mesh)
{
    RectLinkedList *list_iterator = mesh->list;
    Rect *rect;
    while (list_iterator != 0)
    {
        rect = list_iterator->rect;

        // The character isn't even over or under the rect, there is definitely no collision
        if (rect->bottomleft.x <= character->x + character->width)
        {
            if (rect->bottomright.x >= character->x)
            {
                double factor = (character->x - rect->bottomleft.x) / (rect->bottomright.x - rect->bottomleft.x);
                double y_top = rect->topleft.y + factor*(rect->topright.y - rect->topleft.y);
                double y_bottom = rect->bottomleft.y + factor*(rect->bottomright.y - rect->bottomleft.y);
                if (!(character->y <= y_top || character->y - character->height >= y_bottom))
                {
                    return rect;
                }
            }
        }

        list_iterator = list_iterator->next;
    }

    // No collision found
    return (Rect*)0;
}

bool is_connected(Rect *start, Rect *target)
{
    for (int i=0; i<start->num_connections; i++)
    {
        if (start->connections[i] == target)
        {
            return true;
        }
    }
    return false;
}

void add_to_linked_list(RectLinkedList *list, Rect *rect)
{
    RectLinkedList *l = list, *new_linked_list;
    while (l->next != 0)
    {
        l = l->next;
    }
    new_linked_list = calloc(sizeof(RectLinkedList), 1);
    l->next = new_linked_list;
    new_linked_list->rect = rect;
}

void destroy_linked_list(RectLinkedList *start)
{
    RectLinkedList *p1, *p2;
    p1 = start;
    while (p1 != 0)
    {
        p2 = p1->next;
        free(p1);
        p1 = p2;
    }
}

RectLinkedList* copy_linked_list(RectLinkedList *start, RectLinkedList *output)
{
    RectLinkedList *l = calloc(sizeof(RectLinkedList), 1);
    RectLinkedList *tmp = start;

    output = l;

    while (tmp != 0)
    {
        l->rect = tmp->rect;
        l->next = calloc(sizeof(RectLinkedList), 1);
        tmp = tmp->next;
        l = l->next;
    }

    return output;
}

double distance(Rect *r1, Rect *r2)
{
    Point a, b;
    a.x = (r1->bottomleft.x + r1->bottomright.x)/2;
    a.y = (r1->bottomleft.y + r1->topleft.y)/2;

    b.x = (r2->bottomleft.x + r2->bottomright.x)/2;
    b.y = (r2->bottomleft.y + r2->topleft.y)/2;

    return sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
}
