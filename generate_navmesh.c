#include "generate_navmesh.h"
#include "data_types.h"

#define JUMP_HEIGHT 70
#define STAIR_HEIGHT 6
#define MAX_STAIR_WIDTH 11*6

Navmesh* generate_navmesh(Bitmask *map, int char_width, int char_height, double char_speed)
{
    Navmesh *mesh = (Navmesh*) calloc(1, sizeof(Navmesh));

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

    i=0;
    RectLinkedList *tmp = mesh->list;
    if (tmp == NULL)
    {
        mesh->num_rects = 0;
    }
    else
    {
        i++;
        while (tmp->next != NULL)
        {
            tmp = tmp->next;
            i++;
        }
        mesh->num_rects = i;
    }

    return mesh;
}

void add_to_navmesh(Navmesh* mesh, int bottomleft_x, int bottomleft_y, int width, int height)
{
    Rect *r = (Rect*) calloc(1, sizeof(Rect));

    printf("\n\nPosition: (%i | %i), (%i | %i)", bottomleft_x, bottomleft_y, bottomleft_x+width, bottomleft_y+height);

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
}
