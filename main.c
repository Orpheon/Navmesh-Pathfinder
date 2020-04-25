#include "main.h"
#include "move.h"
#include "find_path.h"
#include "load_map.h"
#include "generate_navmesh.h"
#include "navmesh.h"
#include "export_navmesh.h"
#include "data_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define PATH_COMPLETE -1
#define PATH_OUTDATED -2

int last_error = 0;

// ================================================
//                      TODO
// ================================================
//
// Detect possible jumps by walking off, not by jumping up.
// Jumping up means you need to test far too many possibilities across several rectangles.
// Dropping down gives you ideal solutions robustly.
// Only problem might be arial obstacles, but we could deal with those with jumping up once we found a spot.


#ifdef DEBUG_MODE
int main()
{
    Bitmask *map = load_from_file("Maps/ctf_paramental.png");
    printf("\nSize: %i, %i", map->width, map->height);
    int width = 18, height = 35;
    double speed = 4.0;
    Navmesh *mesh = generate_navmesh(map, width, height, speed);
    printf("\n\nNAVMESH GENERATED.");
    export_navmesh(mesh, "ctf_paramental");
    printf("\n\nNAVMESH PRINTED.\n");
    double path = plan_path(ptr_to_gm(mesh), 2964, 876, 1164, 444);
    printf("\nPlan path last error: %i", last_error);
    path = update_position(ptr_to_gm(mesh), path, 2964, 876, 5);
    printf("\nPath pointer: %d", path);
    printf("\nUpdate position last error 2: %i", last_error);
    double input = get_input(path, 2964, 876, 0, 0);
    printf("\nInput: %f", input);
    printf("\n");
}
#endif

double ptr_to_gm(void *p) {
    double d;
    memcpy(&d, &p, sizeof(void*));
    return d;
}

void* gm_to_ptr(double d) {
    void *p;
    memcpy(&p, &d, sizeof(void*));
    return p;
}

__declspec(dllexport) double initialize_mesh(char *mapname)
{
    last_error = 0;
    Bitmask *map = load_from_file(mapname);
    if ((int)map == -1)
    {
        // Map not found
        last_error = -1;
        return 0;
    }
    else if ((int)map == -2)
    {
        // Map contained no wallmask data
        last_error = -2;
        return 0;
    }
    int width = 18, height = 35;
    double speed = 4.0;
    Navmesh *mesh = generate_navmesh(map, width, height, speed);
    return ptr_to_gm(mesh);
}

__declspec(dllexport) double clear_navmesh(double meshptr)
{
    Navmesh *mesh = (Navmesh*) gm_to_ptr(meshptr);
    destroy_navmesh(mesh);
    return 0;
}

__declspec(dllexport) double plan_path(double meshptr, double start_x, double start_y, double target_x, double target_y)
{
    last_error = 0;
    Navmesh *mesh = (Navmesh*) gm_to_ptr(meshptr);
    Rect *start_rect = point_inside_rect(mesh, (int)start_x, (int)start_y);
    Rect *target_rect = point_inside_rect(mesh, (int)target_x, (int)target_y);
    if (start_rect == 0)
    {
        // Our starting point is not inside a rect
        // TODO
        last_error = -1;
        return 0;
    }
    if (target_rect == 0)
    {
        // Our target point is not inside a rect
        // TODO
        last_error = -2;
        return 0;
    }

    RectLinkedList *path = find_path(mesh, start_rect, target_rect);
    return ptr_to_gm(path);
}

__declspec(dllexport) double get_next_rect_x(double pathptr)
{
    RectLinkedList *path = (RectLinkedList*) gm_to_ptr(pathptr);
    return 0.5*(path->next->rect->bottomleft.x + path->next->rect->bottomright.x);
}

__declspec(dllexport) double get_next_rect_y(double pathptr)
{
    RectLinkedList *path = (RectLinkedList*) gm_to_ptr(pathptr);
    return 0.5*(path->next->rect->bottomleft.y + path->next->rect->bottomright.y);
}

__declspec(dllexport) double update_position(double meshptr, double pathptr, double x, double y, double halfwidth)
{
    Navmesh *mesh = (Navmesh*) gm_to_ptr(meshptr);
    last_error = 0;
    RectLinkedList *l;
    Rect* current_rect;
    // widthoffset iterates through 0, halfwidth, and -halfwidth, in that order
    for (int widthoffset = 0; widthoffset <= halfwidth; widthoffset = halfwidth-2*widthoffset)
    {
        l = (RectLinkedList*) gm_to_ptr(pathptr);

        // The most probable situation is that we're inside the old rect
        // If this is the case, no need to do anything
        if (l->rect->bottomleft.x <= (x+widthoffset) && (x+widthoffset) <= l->rect->bottomright.x)
        {
            if (l->rect->topleft.y <= y && y <= l->rect->bottomleft.y)
            {
                // No changes to current path, continue as planned
                return pathptr;
            }
        }
        // Since we want to know what rect if we find one anyway, go through all of them
        current_rect = point_inside_rect(mesh, (x+widthoffset), y);
        // We're inside current_rect
        // First check whether this is within the path
        l = (RectLinkedList*) gm_to_ptr(pathptr);
        l = l->next;
        while (l != 0)
        {
            // If this is the rect we're inside
            if (l->rect == current_rect)
            {
                // Ok, we need to shorten the list up to here
                // Disconnect the list at the last element and destroy the first part
                destroy_linked_list((RectLinkedList*) gm_to_ptr(pathptr), l);
                // Return the beginning of the updated list
                return ptr_to_gm(l);
            }
            l = l->next;
        }

        // We're not in the path, are we in some other rect?
        if (current_rect != 0)
        {
            // Destroy the old path and signal to recreate one
            destroy_linked_list((RectLinkedList*) gm_to_ptr(pathptr), 0);
            last_error = PATH_OUTDATED;
            return 0;
        }
    }

    // If we haven't returned previously, this means we're really in the air
    return pathptr;
}

__declspec(dllexport) double get_input(double pathptr, double x, double y, double hs, double vs)
{
    RectLinkedList *l = (RectLinkedList*) gm_to_ptr(pathptr);
    Character c;
    c.x = x;
    c.y = y;
    c.hs = hs;
    c.vs = vs;
    c.width = 18;
    c.height = 36;
    c.speed = 4.0;
    char *a = get_commands(&c, l->rect, l->next->rect);
    double result;
    if (a[1])
    {
        result = 2*(double)a[0];
    }
    else
    {
        result = (double)a[0];
    }
    free(a);
    return result;
}

__declspec(dllexport) double free_path(double pathptr)
{
    RectLinkedList *path = (RectLinkedList*) gm_to_ptr(pathptr);
    destroy_linked_list(path, 0);
    return 0;
}

__declspec(dllexport) double return_last_error()
{
    return last_error;
}
