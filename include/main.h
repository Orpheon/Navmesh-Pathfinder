#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef DEBUG_MODE
int main();
#endif

double ptr_to_gm(void *p);
void* gm_to_ptr(double d);
__declspec(dllexport) double initialize_mesh(char *mapname);
__declspec(dllexport) double clear_navmesh(double meshptr);
__declspec(dllexport) double plan_path(double meshptr, double start_x, double start_y, double target_x, double target_y);
__declspec(dllexport) double get_next_rect_x(double pathptr);
__declspec(dllexport) double get_next_rect_y(double pathptr);
__declspec(dllexport) double update_position(double meshptr, double pathptr, double x, double y, double halfwidth);
__declspec(dllexport) double get_input(double pathptr, double x, double y, double hs, double vs);
__declspec(dllexport) double free_path(double pathptr);
__declspec(dllexport) double return_last_error();


//#include <windows.h>
//
///*  To use this exported function of dll, include this header
// *  in your project.
// */
//
//#ifdef BUILD_DLL
//    #define DLL_EXPORT __declspec(dllexport)
//#else
//    #define DLL_EXPORT __declspec(dllimport)
//#endif
//
//
//#ifdef __cplusplus
//extern "C"
//{
//#endif
//
//void DLL_EXPORT SomeFunction(const LPCSTR sometext);
//
//#ifdef __cplusplus
//}
//#endif

#endif // __MAIN_H__
