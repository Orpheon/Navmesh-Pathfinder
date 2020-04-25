#ifndef EXPORT_NAVMESH_H_INCLUDED
#define EXPORT_NAVMESH_H_INCLUDED

#include "data_types.h"

void export_navmesh(Navmesh *mesh, char* name);

int find_index(Navmesh *mesh, Rect *rect);

#endif // EXPORT_NAVMESH_H_INCLUDED
