#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include "test.h"
#include "crt.h"
#include "qtree.h"

static void test_tree() {
    DESCRIBE("tree");
    QuadTree *tree;
    QuadNode *root;

    tree = qtree_create((Vec2){0}, (Vec2) {600.f, 400.f});
    assert(tree->length == 0);

    root = tree->root;
    assert(!qnode_isleaf(root));
    assert(qnode_isempty(root));
    assert(!qnode_ispointer(root));

    assert(root->self_nw.x == 0);
    assert(root->self_nw.y == 0);
    assert(root->self_se.x == 600.0);
    assert(root->self_se.y == 400.0);

    assert(root->self_width == 600.0);
    assert(root->self_height == 400.0);

    qtree_destroy(tree);
    DONE();
}

static void test_node() {
    DESCRIBE("node");
    QuadNode *node;

    node = qnode_create(NULL);

    assert(!qnode_isleaf(node));
    assert(qnode_isempty(node));
    assert(!qnode_ispointer(node));
    assert(node->parent == NULL);

    qnode_destroy(node);
    DONE();
}

static void test_node_bounds() {
    DESCRIBE("bounds");
    QuadNode *node;

    node = qnode_create(NULL);

    qnode_set_bounds(node, (Vec2) {1.f, 2.f}, (Vec2) {2.f, 4.f});
    assert(node->self_nw.x == 1.0);
    assert(node->self_nw.y == 2.0);
    assert(node->self_se.x == 2.0);
    assert(node->self_se.y == 4.0);

    assert(node->self_width == 1.0);
    assert(node->self_height == 2.0);

    qnode_destroy(node);
    DONE();
}

static void test_tree_insert() {
    QuadTree *tree =  qtree_create((Vec2){1.f, 1.f}, (Vec2) {10.f, 10.f});
    assert(tree != NULL);
    assert(tree->root->self_nw.x == 1.0);
    assert(tree->root->self_nw.y == 1.0);
    assert(tree->root->self_se.x == 10.0);
    assert(tree->root->self_se.y == 10.0);

    Creature *crt1 = crt_create(111);
    Creature *crt2 = crt_create(222);

    crt1->pos = (Vec2) {8.f, 2.f};
    crt2->pos = (Vec2) {1.f, 1.f};

    {
        DESCRIBE("test_qtree_insert(first node)");
        int res = qtree_insert(tree, crt1);
        // qnode_print(tree->root);

        assert(tree->root->crt != NULL);
        assert(res == QUAD_INSERTED);
        assert(tree->length == 1);

        assert(tree->root->nw == NULL);
        assert(tree->root->ne == NULL);
        assert(tree->root->se == NULL);
        assert(tree->root->sw == NULL);

        // verify node idendity
        assert(tree->root->crt->id == crt1->id);
        DONE();
    } {
        DESCRIBE("test_qtree_insert(second node)");
        int res = qtree_insert(tree, crt2);
        // qnode_print(tree->root);

        assert(tree->root->crt == NULL); // 111 has been moved
        assert(res == QUAD_INSERTED);
        assert(tree->length == 2);

        // splitting
        assert(tree->root->nw != NULL);
        assert(tree->root->ne != NULL);
        assert(tree->root->se != NULL);
        assert(tree->root->sw != NULL);

        // verify node idendity
        assert(tree->root->ne->crt->id == crt1->id);
        DONE();
    }

    qtree_destroy(tree);
}

static void test_tree_insert_outside() {
    DESCRIBE("pos outside of tree");
    QuadTree *tree = qtree_create((Vec2){1.f, 1.f}, (Vec2) {10.f, 10.f});

    Creature *crt = crt_create(111);
    crt->pos = (Vec2) {0};

    int res;
    res = qtree_insert(tree, crt);

    assert(tree->root->crt == NULL);
    assert(res == QUAD_FAILED);

    qtree_destroy(tree);
    DONE();
}


static void test_tree_insert_replace() {
    DESCRIBE("replace if (n2.pos == n1.pos)");
    QuadTree *tree = qtree_create((Vec2){1.f, 1.f}, (Vec2) {10.f, 10.f});

    Creature *crt1 = crt_create(111);
    Creature *crt2 = crt_create(222);
    crt1->pos = (Vec2) {8.f, 2.f};
    crt2->pos = (Vec2) {8.f, 2.f};

    int res;

    {
        // first node
        res = qtree_insert(tree, crt1);

        assert(res == QUAD_INSERTED);
        assert(tree->length == 1);
        assert(tree->root->crt != NULL);
        assert(tree->root->crt->id == crt1->id);
    } {
        // second node replaces first
        res = qtree_insert(tree, crt2);

        assert(res == QUAD_REPLACED);
        assert(tree->length == 1);
        assert(tree->root->crt != NULL);
        assert(tree->root->crt->id == crt2->id);

        // splitting
        {
            assert(tree->root->nw == NULL);
            assert(tree->root->ne == NULL);
            assert(tree->root->se == NULL);
            assert(tree->root->sw == NULL);
        }
    }

    qtree_destroy(tree);
    DONE();
}

static void test_tree_find() {
    DESCRIBE("find");
    QuadTree *tree = qtree_create((Vec2){1.f, 1.f}, (Vec2) {10.f, 10.f});

    Creature *crt1 = crt_create(111);
    Creature *crt2 = crt_create(222);
    crt1->pos = (Vec2) {8.f, 2.f};
    crt2->pos = (Vec2) {1.f, 1.f};

    QuadNode *node;
    int res;
    Vec2 search;

    {
        // insert nodes
        res = qtree_insert(tree, crt1);
        assert(res == QUAD_INSERTED);

        res = qtree_insert(tree, crt2);
        assert(res == QUAD_INSERTED);

        assert(tree->length == 2);
    } {
        // find second non-existing node;
        search = (Vec2){0};
        node = qtree_find(tree, search);
        assert(node == NULL);
    } {
        // find second item;
        search = crt2->pos;
        node = qtree_find(tree, search);
        assert(node != NULL);
        assert(node->crt->id == crt2->id);
    } {
       //find first item
        search = crt1->pos;
        node = qtree_find(tree, search);
        assert(node != NULL);
        assert(node->crt->id == crt1->id);
    }

    qtree_destroy(tree);
    DONE();
}

static void test_node_parent() {
    DESCRIBE("parent");
    QuadTree *tree = qtree_create((Vec2){1.f, 1.f}, (Vec2) {10.f, 10.f});

    Creature *crt1 = crt_create(111);
    Creature *crt2 = crt_create(222);

    // a nested tree with three levels
    crt1->pos = (Vec2) {8.f, 2.f};
    crt2->pos = (Vec2) {9.f, 1.f};

    QuadNode *node, *parent;
    int res;
    Vec2 search;

    {
        // insert nodes
        res = qtree_insert(tree, crt1);
        assert(res == QUAD_INSERTED);

        res = qtree_insert(tree, crt2);
        assert(res == QUAD_INSERTED);

        assert(tree->length == 2);
    } {
        // find first parent
        parent = tree->root;
        node = tree->root->ne;

        assert(node != NULL);
        assert(node->parent != NULL);

        assert(node->parent->self_nw.x == parent->self_nw.x);
        assert(node->parent->self_se.y == parent->self_se.y);
    } {
        // find second parent
        parent = tree->root->ne;
        node   = tree->root->ne->ne;

        assert(node != NULL);
        assert(node->parent != NULL);

        assert(node->parent->self_nw.x == parent->self_nw.x);
        assert(node->parent->self_se.y == parent->self_se.y);

        //qnode_print(stderr, node);
    } {
        // find first leaf
        parent = tree->root->ne->ne;
        node   = tree->root->ne->ne->nw;

        assert(node != NULL);
        assert(node->parent != NULL);

        assert(node->parent->self_nw.x == parent->self_nw.x);
        assert(node->parent->self_se.y == parent->self_se.y);

        // crt
        assert(node->crt != NULL);
        assert(node->crt->id == crt1->id);

        //qnode_print(stderr, node);
    } {
        // find second leaf
        parent = tree->root->ne->ne;
        node   = tree->root->ne->ne->ne;

        assert(node != NULL);
        assert(node->parent != NULL);

        assert(node->parent->self_nw.x == parent->self_nw.x);
        assert(node->parent->self_se.y == parent->self_se.y);

        // crt
        assert(node->crt != NULL);
        assert(node->crt->id == crt2->id);

        //qnode_print(stderr, node);
    }

    qtree_destroy(tree);
    DONE();
}

static void test_node_qtree_find_in_area() {
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

    CrtList *list = crt_list_create(1);
    int res, i;

    {
        res = qtree_insert(tree, ref);
        assert(res == QUAD_INSERTED);

        // insert nodes
        for (i = 0; i < 3; i++) {
            res = qtree_insert(tree, inside[i]);
            assert(res == QUAD_INSERTED);
            //printf("(%d): {%f, %f}, nw: {%f, %f}, se: {%f, %f}\n", inside[i]->id, inside[i]->pos.x, inside[i]->pos.y, nw.x, nw.y, se.x, se.y);
        }
        for (i = 0; i < 3; i++) {
            res = qtree_insert(tree, outside[i]);
            printf(" -- %d\n", res);
            printf("(%d): {%f, %f}, nw: {%f, %f}, se: {%f, %f}\n", outside[i]->id, outside[i]->pos.x, outside[i]->pos.y, nw.x, nw.y, se.x, se.y);
            assert(res == QUAD_INSERTED);
        }
        assert(tree->length == 7); // 2 * 3 + 1
    } {
        qtree_find_in_area(tree, ref, list, nw, se);
        assert(list != NULL);
        crt_list_print(stderr, list);
        assert(list->len == 3);

    }

    crt_list_destroy(list);
    qtree_destroy(tree);
    DONE();
}

void test_qtree(int argc, char **argv) {
    test_tree();
    test_node();
    test_node_bounds();
    test_tree_insert();
    test_tree_insert_outside();
    test_tree_insert_replace();
    test_tree_find();
    test_node_parent();
    test_node_qtree_find_in_area();
    return;
}
