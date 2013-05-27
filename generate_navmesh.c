#include "generate_navmesh.h"
#include "data_types.h"

#define JUMP_HEIGHT 70
#define STAIR_HEIGHT 6
#define MAX_STAIR_WIDTH 11*6

Navmesh* generate_navmesh(Bitmask *map, int char_width, int char_height, double char_speed)
{
    Navmesh *mesh = (Navmesh*) calloc(1, sizeof(Navmesh));
    Rect *rect;
    RectLinkedList *list_iterator;

    printf("---GENERATING AREAS---");

    // Generate all the areas, including unnecessary ones
    int x, y, i, j, max_height;
    for (y=0; y<map->height-1; y++)
    {
        x = 0;
        while (x < map->width-1)
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
    while (list_iterator != NULL)
    {
        rect = list_iterator->rect;
        rect->topleft.y += char_height;
        rect->topright.y += char_height;
        if (rect->topleft.y >= rect->bottomleft.y)
        {
            // This rect was too low for a character to fit through.
            // Destroy the rect
            remove_from_navmesh(mesh, rect);
        }
        list_iterator = list_iterator->next;
    }

    // Stair optimisations
    // Remove and simplify all rects that are clearly steps of linear stairs
    printf("\n---OPTIMIZING STAIRS---");
    list_iterator = mesh->list;
    i = 0;
    while (list_iterator != NULL)
    {
        rect = list_iterator->rect;
        // The rectangle would only be part of a stair if its width is the width of every step
        int stair_width = rect->bottomright.x - rect->bottomleft.x + 1;
        if (stair_width >= 1 && stair_width <= MAX_STAIR_WIDTH)
        {
            Rect *next_rect = find_rect(mesh, rect->bottomleft.x-stair_width, rect->bottomleft.y+STAIR_HEIGHT);
            if (next_rect != NULL)
            {
                // Stair is going left and down
                while (next_rect != NULL)
                {
                    // Check whether the stair continues
                    if (next_rect->bottomright.x - next_rect->bottomleft.x + 1 != stair_width)
                    {
                        break;
                    }
                    if (rect->topleft.y - rect->bottomleft.y != next_rect->topright.y - next_rect->bottomright.y)
                    {
                        break;
                    }

                    // As long as steps are where they should, merge them in the original one
                    rect->bottomleft.x = next_rect->bottomleft.x;
                    rect->bottomleft.y = next_rect->bottomleft.y;
                    rect->topleft.x = next_rect->topleft.x;
                    rect->topleft.y = next_rect->topleft.y;
                    remove_from_navmesh(mesh, next_rect);
                    next_rect = find_rect(mesh, rect->bottomleft.x-stair_width, rect->bottomleft.y+STAIR_HEIGHT);
                }
            }
            else
            {
                // Same as above, only this time for stairs that go right and down
                x = rect->bottomleft.x;
                y = rect->bottomleft.y;
                next_rect = find_rect(mesh, x+stair_width, y+STAIR_HEIGHT);
                while (next_rect != NULL)
                {
                    // Check whether the stair continues
                    if (next_rect->bottomright.x - next_rect->bottomleft.x + 1 != stair_width)
                    {
                        break;
                    }
                    if (rect->topright.y - rect->bottomright.y != next_rect->topleft.y - next_rect->bottomleft.y)
                    {
                        break;
                    }

                    rect->bottomright.x = next_rect->bottomright.x;
                    rect->bottomright.y = next_rect->bottomright.y;
                    rect->topright.x = next_rect->topright.x;
                    rect->topright.y = next_rect->topright.y;
                    remove_from_navmesh(mesh, next_rect);
                    x += stair_width;
                    y += STAIR_HEIGHT;
                    next_rect = find_rect(mesh, x+stair_width, y+STAIR_HEIGHT);
                }
            }
        }

        list_iterator = list_iterator->next;
    }

    return mesh;
}

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
    link->next = NULL;

    RectLinkedList *tmp = mesh->list;
    if (tmp == NULL)
    {
        mesh->list = link;
    }
    else
    {
        while (tmp->next != NULL)
        {
            tmp = tmp->next;
        }
        tmp->next = link;
    }

    mesh->num_rects++;
}

Rect* find_rect(Navmesh *mesh, double x, double y)
{
    // Returns a rect if there's one with the bottomleft corner at that position, NULL if there isn't
    RectLinkedList *l = mesh->list;
    Rect *r;

    while (l != NULL)
    {
        r = l->rect;
        if (r->bottomleft.x == x && r->bottomleft.y == y)
        {
            return r;
        }
        l = l->next;
    }
    return NULL;
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
    while (l->next != NULL)
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
