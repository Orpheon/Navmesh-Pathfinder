project(NavmeshPathfinder)
cmake_minimum_required(VERSION 2.8)

include_directories("${PROJECT_SOURCE_DIR}/include")

set(SRC_FILES
        main.c
        load_map.c
        generate_navmesh.c
        find_path.c
        export_navmesh.c
        move.c
        navmesh.c
        simulation.c)

SET( CMAKE_C_FLAGS  "${CMAKE_CXX_FLAGS} -fdeclspec" )

add_library(navmesh ${SRC_FILES})