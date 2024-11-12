#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include "test.h"
#include "qtree.h"

typedef struct TestItem {
    int id;
    Vec2 pos; // control data, will not be queried within qtree.h
} TestItem;


static int _in_list(int id, QuadList *list) {
    for (size_t i = 0; i < list->len; i++) {
        if (list->nodes[i] && list->nodes[i]->data) {
            TestItem *item = (TestItem*) list->nodes[i]->data;
            if (item->id == id) {
                return 1;
            }
        }
    }
    return 0;
}


static void test_qnode_within_area() {
    DESCRIBE("node covered by area");

    int res;
    QuadNode *node = qnode_create(NULL);
    node->self_nw = (Vec2) {2.f, 2.f};
    node->self_se = (Vec2) {5.f, 5.f};

    // outside area
    res = qnode_within_area(node, (Vec2) {0.f, 0.f}, (Vec2) {1.f, 1.f});
    assert(res == 0);

    // overlaps area
    res = qnode_within_area(node, (Vec2) {0.f, 0.f}, (Vec2) {3.f, 3.f});
    assert(res == 0);

    // exact coverage
    res = qnode_within_area(node, (Vec2){2.f, 2.f}, (Vec2) {5.f, 5.f});
    assert(res == 1);

    // inside area
    res = qnode_within_area(node, (Vec2){1.f, 1.f}, (Vec2) {6.f, 6.f});
    assert(res == 1);

    qnode_destroy(node);
    DONE();
}

static void test_qnode_overlaps_area() {
    DESCRIBE("node overlaps area");

    int res;
    QuadNode *node = qnode_create(NULL);
    node->self_nw = (Vec2) {2.f, 2.f};
    node->self_se = (Vec2) {5.f, 5.f};

    // outside area
    res = qnode_overlaps_area(node, (Vec2) {0.f, 0.f}, (Vec2) {1.f, 1.f});
    assert(res == 0);

    // overlaps area
    res = qnode_overlaps_area(node, (Vec2) {0.f, 0.f}, (Vec2) {3.f, 3.f});
    assert(res == 1);

    // exact coverage
    res = qnode_overlaps_area(node, (Vec2){2.f, 2.f}, (Vec2) {5.f, 5.f});
    assert(res == 1);

    // inside area
    res = qnode_overlaps_area(node, (Vec2){1.f, 1.f}, (Vec2) {6.f, 6.f});
    assert(res == 1);

    qnode_destroy(node);
    DONE();
}

static void test_find_in_area() {
    DESCRIBE("area");
    QuadTree *tree = qtree_create((Vec2){1.f, 1.f}, (Vec2) {10.f, 10.f});

    float radius = 2.f;
    Vec2 pos = (Vec2) {4.f, 4.f};

    TestItem ref = {0, pos};

    Vec2 nw = {ref.pos.x - radius, ref.pos.y - radius};
    Vec2 se = {ref.pos.x + radius, ref.pos.y + radius};
    // printf("ref: {%f, %f}, nw: {%f, %f}, se: {%f, %f}\n", ref.pos.x, ref.pos.y, nw.x, nw.y, se.x, se.y);

    // inside
    TestItem itm1 = {
        .id=1,
        .pos=(Vec2) {pos.x + (radius / 2.f), pos.y + (radius / 2.f)}
    };

    TestItem itm2 = {
        .id=2,
        .pos=(Vec2) {pos.x + radius, pos.y + radius}
    };

    TestItem itm3 = {
        .id=3,
        .pos=(Vec2) {pos.x - radius, pos.y - radius}
    };

    // outside
    TestItem itm4 = {
        .id=4,
        .pos=(Vec2) {1.f, 1.f}
    };

    TestItem itm5 = {
        .id=5,
        .pos=(Vec2) {pos.x, pos.y + radius + 0.1}
    };

    TestItem itm6 = {
        .id=6,
        .pos=(Vec2) {pos.x - radius - 0.1, pos.y}
    };

    TestItem *inside[3]  = {&itm1, &itm2, &itm3};
    TestItem *outside[3] = {&itm4, &itm5, &itm6};

    QuadList *list = qlist_create(1);
    int res, i;

    res = qtree_insert(tree, &ref, ref.pos);
    assert(res == QUAD_INSERTED);

    // insert nodes
    for (i = 0; i < 3; i++) {
        res = qtree_insert(tree, inside[i], inside[i]->pos);
        // printf("(%d): {%f, %f}, nw: {%f, %f}, se: {%f, %f}\n", inside[i]->id, inside[i]->pos.x, inside[i]->pos.y, nw.x, nw.y, se.x, se.y);
        assert(res == QUAD_INSERTED);
    }
    for (i = 0; i < 3; i++) {
        res = qtree_insert(tree, outside[i], outside[i]->pos);
        // printf("(%d): {%f, %f}, nw: {%f, %f}, se: {%f, %f}\n", outside[i]->id, outside[i]->pos.x, outside[i]->pos.y, nw.x, nw.y, se.x, se.y);
        assert(res == QUAD_INSERTED);
    }
    assert(tree->length == 7); // 2 * 3 + 1

    // qtree_print(stdout, tree); exit(0);
    qtree_find_in_area(tree, pos, radius, list);

    assert(list != NULL);
    assert(list->len == 4);

    assert(_in_list(0, list));
    assert(_in_list(1, list));
    assert(_in_list(2, list));
    assert(_in_list(3, list));

    qlist_destroy(list);
    qtree_destroy(tree);
    DONE();
}

void test_qtree_area(int argc, char **argv) {
    test_qnode_within_area();
    test_qnode_overlaps_area();

    test_find_in_area();
}
