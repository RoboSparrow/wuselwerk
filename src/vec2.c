#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "vec2.h"

// @see https://www.gnu.org/software/libc/manual/html_node/Mathematical-Constants.html
#define _USE_MATH_DEFINES // math.h on older systems

#ifndef M_PI
#define M_PI 3.14159265359
#endif

const float VEC2_TWO_PI = M_PI * 2;

/**
 * Create a copy of a vector
 */
Vec2 vec2_copy(Vec2 v) {
    return (Vec2){v.x, v.y};
}

/**
 * Adds two vectors
 */
Vec2 vec2_add(Vec2 v, Vec2 a) {
    return (Vec2){v.x + a.x, v.y + a.y};
}

/**
 * Subtracts two vectors
 */
Vec2 vec2_sub(Vec2 v, Vec2 s) {
    return (Vec2){v.x - s.x, v.y - s.y};
}

/**
 * Computes length (or magnitude) of a vector
 */
float vec2_mag(Vec2 v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

/**
 * Normalizes a vector
 */
Vec2 vec2_norm(Vec2 v) {
    float mag = vec2_mag(v);
    if (mag == 0) {
        return (Vec2){0};
    }
    return (Vec2){v.x / mag, v.y / mag};
}

/**
 * Computes the distance between two vectors
 */
float vec2_dist(Vec2 v, Vec2 d) {
    Vec2 c = vec2_sub(v, d);
    return vec2_mag(c);
}

/**
 * Check two vectors for equality
 */
int vec2_equals(Vec2 l, Vec2 r) {
    return (l.x == r.x && l.y == r.y);
}

/**
 * checks if a point is inside a given area
 */
int vec2_within(Vec2 pos, Vec2 nw, Vec2 se) {
    return pos.x >= nw.x && pos.y >= nw.y && pos.x <= se.x && pos.y <= se.y;
}

/**
 * Creates a random new vector within a radius RELATIVE from a source vector (2D!)
 * -x, y axis only (2d)
 */
Vec2 vec2_rand_from(Vec2 pos, float radius) {
    PVec2 p = {
        .r = (float)rand_range_f(-1 * radius, radius),
        .phi = rand_range_f(0, VEC2_TWO_PI) // radians
    };
    return vec2_polar_to_cartesian(p);
}

/**
 * Moves Vec2 from one step nearer to Vec2 to
 */
Vec2 vec2_move_to(Vec2 from, Vec2 to, float speed) {
    Vec2 delta = vec2_sub(to, from);
    float mag = vec2_mag(delta);

    // overshoot
    if (mag < (speed * speed)) {
        return vec2_copy(to);
    }

    Vec2 res = vec2_copy(from);
    Vec2 norm = vec2_norm(delta);

    res.x = res.x + speed * norm.x;
    res.y = res.y + speed * norm.y;
    return res;
}

/**
 * Converts a polar vector to a cartesian vector
 */
Vec2 vec2_polar_to_cartesian(PVec2 p) {
    Vec2 v = {0};
    v.x = p.r * cosf(p.phi);
    v.y = p.r * sinf(p.phi);
    return v;
}

/**
 * Converts a cartesian vector to a polar vector
 */
PVec2 vec2_cartesian_to_polar(Vec2 v) {
    PVec2 p = {0};
    p.r = sqrtf(v.x * v.x + v.y * v.y);
    p.phi = atan2f(v.y, v.x);
    return p;
}
