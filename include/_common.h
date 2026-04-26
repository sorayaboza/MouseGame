#pragma GCC diagnostic ignored "-Wattributes"
#ifndef _COMMON_H
#define _COMMON_H

#include<string>
#include<GL/glew.h>
#include <windows.h>
#include <iostream>

#include <stdlib.h>
#include <gl/gl.h>
#include <GL/glut.h>
#include <time.h>
#include <math.h>
#include <chrono>

#define PI 3.14159
#define GLEW_STATIC

#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

using namespace std;

typedef struct{
   float x;
   float y;
   float z;
}vec3;

typedef struct{
   float x;
   float y;
}vec2;

#endif // _COMMON_H
