#ifndef __VEC2_H__
#define __VEC2_H__

#include <math.h>


/**
 * Cartesian
 */

typedef struct Vec2 {
    float x;
    float y;
} Vec2;

#define VEC2_PRINT(fp,v) fprintf(fp, "{x:%.2f, y:%.2f}", v.x, v.y)

// core

Vec2 vec2_copy(Vec2 v);
Vec2 vec2_add(Vec2 v, Vec2 s);
Vec2 vec2_sub(Vec2 v, Vec2 s);
Vec2 vec2_norm(Vec2 v);

float vec2_mag(Vec2 v);
float vec2_dist(Vec2 v, Vec2 d);

int vec2_equals(Vec2 l, Vec2 r);

// actions

Vec2 vec2_rand_from(Vec2 pos, float radius);
Vec2 vec2_move_to(Vec2 from, Vec2 to, float speed);

/**
 * Polar
 */

typedef struct PVec2 {
    float r;
    float phi;
} PVec2;

#define PVEC2_PRINT(fp,v) fprintf(fp, "{r:%f, phi:%f}", v.r, v.phi)

/**
 * conversions
 */

Vec2 vec2_polar_to_cartesian(PVec2 p);
PVec2 vec2_cartesian_to_polar(Vec2 v);


#endif
