#include "main.h"
#include "load_map.c"
#include "generate_navmesh.c"
#include "export_navmesh.c"
#include <stdio.h>

int main()
{
    Bitmask *map = load_from_file("Maps/ctf_dirtbowl_v2.png");
    char str[map->width * (map->height+1)];
    int k=0;

    for (int j=0; j<map->height; j++)
    {
        for (int i=0; i<map->width; i++)
        {
            if (map->mask[i][j])
            {
                str[k++] = '$';
            }
            else
            {
                str[k++] = ' ';
            }
        }
        str[k++] = '\n';
    }

    FILE *f;
    f = fopen("output_map", "w");
    fprintf(f, str);
//    printf("\nSize: %i, %i", map->width, map->height);
//    int width = 18, height = 35;
//    double speed = 2.0;
//    Navmesh *mesh = generate_navmesh(map, width, height, speed);
//    export_navmesh(mesh, "ctf_dirtbowl_v2");
}
/*
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}
*/
