#include "export_navmesh.h"
#include "data_types.h"
#include <stdio.h>

void export_navmesh(Navmesh *mesh, char* name)
{
    char filename[strlen(name)+strlen(".navmesh")];
    strcpy(filename, name);
    strcat(filename, ".navmesh");
    printf(filename);
    FILE *f;
    f = fopen(filename, "w");

    fwrite((const void*) &(mesh->num_rects), sizeof(int), 1, f);
    printf("\n%i", mesh->num_rects);
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

//    int a[mesh->num_rects];
//    for (int i=0; i<mesh->num_rects; i++)
//    {
//        a[i] = 0;
//    }
//    fwrite((const void*) a, sizeof(int), mesh->num_rects, f);
    fclose(f);
    /*l = mesh->list;
    for (int i=0; i<mesh->num_rects; i++)
    {
        r = l->rect;
        fwrite(&(r->num_connections), sizeof(int), 1, f);
        for (int j=0; j<r->num_connections; j++)
        {
            fwrite(find_index(mesh, r->connections[j]), sizeof(int), 1, f);
        }
    }*/
}

int find_index(Navmesh *mesh, Rect *rect)
{
    int i = 0;
    RectLinkedList *l;
    l = mesh->list;
    if (l == 0)
    {
        return -1;
    }
    while (l->next != 0)
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
