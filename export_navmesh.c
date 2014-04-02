#include "export_navmesh.h"
#include "data_types.h"
#include <stdio.h>
#include <string.h>

void export_navmesh(Navmesh *mesh, char* name)
{
    char filename[strlen(name)+strlen(".navmesh")];
    strcpy(filename, name);
    strcat(filename, ".navmesh");
    FILE *f;
    f = fopen(filename, "w");

    fwrite((const void*) &(mesh->num_rects), sizeof(int), 1, f);
    RectLinkedList *l;
    Rect *r;
    l = mesh->list;
    for (int i=0; i<mesh->num_rects; i++)
    {
        r = l->rect;
        // This copies both the x and the y value
        fwrite((const void*) &(r->topleft.x), sizeof(int), 2, f);
        fwrite((const void*) &(r->bottomleft.x), sizeof(int), 2, f);
        fwrite((const void*) &(r->topright.x), sizeof(int), 2, f);
        fwrite((const void*) &(r->bottomright.x), sizeof(int), 2, f);
        l = l->next;
    }

    l = mesh->list;
    int tmp;
    for (int i=0; i<mesh->num_rects; i++)
    {
        r = l->rect;
        fwrite(&(r->num_connections), sizeof(int), 1, f);
        for (int j=0; j<r->num_connections; j++)
        {
            tmp = find_index(mesh, r->connections[j]);
            fwrite((const void*) &tmp, sizeof(int), 1, f);
        }
        l = l->next;
    }
    fclose(f);
}

int find_index(Navmesh *mesh, Rect *rect)
{
    int i = 0;
    RectLinkedList *l;
    l = mesh->list;
    while (l != NULL)
    {
        if (l->rect == rect)
        {
            return i;
        }
        i++;
        l = l->next;
    }
    return -1;
}
