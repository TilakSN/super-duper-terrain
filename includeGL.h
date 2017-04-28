#pragma once

#ifdef _WIN32
    #include <windows.h>
#endif // _WIN32

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glut.h>

#define BUFFER_OFFSET(offset) (GLvoid *)(offset)