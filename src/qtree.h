/**
 *
 * Quadtree implementation based on the execellent quatree library by Jeff Larson (@thejefflarson)
 *
 * @see https://github.com/thejefflarson/quadtree
 *
 */

#ifndef __QTREE_H__
#define __QTREE_H__

#include "crt.h"
#include "vec2.h"

#define QUAD_FAILED -1
#define QUAD_INSERTED 0
#define QUAD_REPLACED 2


////
//   Quadrants
//
//   nw(x,y)
//   x────────────┬────────────┐
//   │            │            │
//   │            │            │
//   │     nw     │     ne     │
//   │            │            │
//   │            │            │
//   ├────────────x────────────┤
//   │            │ c(x,y)     │
//   │            │            │
//   │     sw     │     se     │
//   │            │            │
//   │            │            │
//   └────────────┴────────────x
//                        se(x,y)
////

typedef struct QuadNode {
    struct QuadNode *ne;
    struct QuadNode *nw;
    struct QuadNode *se;
    struct QuadNode *sw;

    Vec2 self_nw;
    Vec2 self_se;

    float self_width;
    float self_height;

    Creature *crt;
} QuadNode;

typedef struct QuadTree {
    QuadNode *root;
    unsigned int length;
} QuadTree;


QuadTree *qtree_create(Vec2 window_nw, Vec2 window_se);
void qtree_destroy(QuadTree *tree);

int qtree_insert(QuadTree *tree, Creature *crt);
Creature *qtree_find(QuadTree *tree, Vec2 pos);

QuadNode *qnode_create();
void qnode_destroy(QuadNode *node);

int qnode_isempty(QuadNode *node);
int qnode_isleaf(QuadNode *node);
int qnode_ispointer(QuadNode *node);

void qnode_set_bounds(QuadNode *node, Vec2 nw, Vec2 se);
void qnode_walk(QuadNode *node, void (*descent)(QuadNode *node), void (*ascent)(QuadNode *node)) ;

void qtree_print(QuadTree *tree);
void qnode_print(QuadNode *node);

#endif
