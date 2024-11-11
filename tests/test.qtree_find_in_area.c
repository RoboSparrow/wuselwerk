#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include "test.h"
#include "crt.h"
#include "qtree.h"

static void test_area() {
    DESCRIBE("area");
    QuadTree *tree = qtree_create((Vec2){1.f, 1.f}, (Vec2) {11.f, 11.f});

    float rad = 2.f;

    Creature *ref = crt_create(0);
    ref->pos = (Vec2) {8.f, 4.f};

    Vec2 nw = {ref->pos.x - rad, ref->pos.y - rad};
    Vec2 se = {ref->pos.x + rad, ref->pos.y + rad};
    // printf("ref: {%f, %f}, nw: {%f, %f}, se: {%f, %f}\n", ref->pos.x, ref->pos.y, nw.x, nw.y, se.x, se.y);

    // inside
    Creature *crt1 = crt_create(1);
    crt1->pos = (Vec2) {8.f, 3.f};

    Creature *crt2 = crt_create(2);
    crt2->pos = (Vec2) {nw.x, nw.y};

    Creature *crt3 = crt_create(3);
    crt3->pos = (Vec2) {se.x, se.y};

    // outside
    Creature *crt4 = crt_create(4);
    crt4->pos = (Vec2) {1.f, 1.f};

    Creature *crt5 = crt_create(5);
    crt5->pos = (Vec2) {ref->pos.x, nw.y - 0.1};

    Creature *crt6 = crt_create(6);
    crt6->pos = (Vec2) {nw.x - 0.1, ref->pos.y};

    Creature *inside[3] = {crt1, crt2, crt3};
    Creature *outside[3] = {crt4, crt5, crt6};

    QuadList *list = qlist_create(1);
    int res, i;

    {
        res = qtree_insert(tree, ref);
        assert(res == QUAD_INSERTED);

        // insert nodes
        for (i = 0; i < 3; i++) {
            res = qtree_insert(tree, inside[i]);
            assert(res == QUAD_INSERTED);
            // printf("(%d): {%f, %f}, nw: {%f, %f}, se: {%f, %f}\n", inside[i]->id, inside[i]->pos.x, inside[i]->pos.y, nw.x, nw.y, se.x, se.y);
        }
        for (i = 0; i < 3; i++) {
            res = qtree_insert(tree, outside[i]);
            // printf("(%d): {%f, %f}, nw: {%f, %f}, se: {%f, %f}\n", outside[i]->id, outside[i]->pos.x, outside[i]->pos.y, nw.x, nw.y, se.x, se.y);
            assert(res == QUAD_INSERTED);
        }
        assert(tree->length == 7); // 2 * 3 + 1
    } {
        qtree_find_in_area(tree, list, nw, se);
        assert(list != NULL);
        // qlist_print(stderr, list);
        assert(list->len == 3);
    }

    qlist_destroy(list);
    qtree_destroy(tree);
    DONE();
}


void test_qtree_find_in_area(int argc, char **argv) {
    test_area();
}
