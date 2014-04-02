#include "main.h"
#include "move.h"
#include "find_path.h"
#include "load_map.h"
#include "generate_navmesh.h"
#include "export_navmesh.h"
#include "data_types.h"
#include <stdio.h>

int main()
{
    Bitmask *map = load_from_file("Maps/ctf_dirtbowl_v2.png");
    printf("\nSize: %i, %i", map->width, map->height);
    int width = 18, height = 35;
    double speed = 2.0;
    Navmesh *mesh = generate_navmesh(map, width, height, speed);
    export_navmesh(mesh, "ctf_dirtbowl_v2");
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
