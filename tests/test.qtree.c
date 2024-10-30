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

    tree = quad_tree_create((Vec2){0}, (Vec2) {600.f, 400.f});
    assert(tree->length == 0);

    root = tree->root;
    assert(!quad_node_isleaf(root));
    assert(quad_node_isempty(root));
    assert(!quad_node_ispointer(root));

    assert(root->self_nw.x == 0);
    assert(root->self_nw.y == 0);
    assert(root->self_se.x == 600.0);
    assert(root->self_se.y == 400.0);

    assert(root->self_width == 600.0);
    assert(root->self_height == 400.0);

    quad_tree_destroy(tree);
    DONE();
}

static void test_node() {
    DESCRIBE("node");
    QuadNode *node;

    node = quad_node_create();

    assert(!quad_node_isleaf(node));
    assert(quad_node_isempty(node));
    assert(!quad_node_ispointer(node));

    quad_node_destroy(node);
    DONE();
}

static void test_node_bounds() {
    DESCRIBE("bounds");
    QuadNode *node;

    node = quad_node_create();

    quad_node_set_bounds(node, (Vec2) {1.f, 2.f}, (Vec2) {2.f, 4.f});
    assert(node->self_nw.x == 1.0);
    assert(node->self_nw.y == 2.0);
    assert(node->self_se.x == 2.0);
    assert(node->self_se.y == 4.0);

    assert(node->self_width == 1.0);
    assert(node->self_height == 2.0);

    quad_node_destroy(node);
    DONE();
}

static void test_quad_tree_insert() {
    QuadTree *tree =  quad_tree_create((Vec2){1.f, 1.f}, (Vec2) {10.f, 10.f});
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
        DESCRIBE("test_quad_tree_insert(first node)");
        int res = quad_tree_insert(tree, crt1);
        // quad_node_print(tree->root);

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
        DESCRIBE("test_quad_tree_insert(second node)");
        int res = quad_tree_insert(tree, crt2);
        // quad_node_print(tree->root);

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

    quad_tree_destroy(tree);
}

static void test_quad_tree_insert_outside() {
    DESCRIBE("pos outside of tree");
    QuadTree *tree = quad_tree_create((Vec2){1.f, 1.f}, (Vec2) {10.f, 10.f});

    Creature *crt = crt_create(111);
    crt->pos = (Vec2) {0};

    int res;

    printf("  - pos outside bounds (nw)\n");
    res = quad_tree_insert(tree, crt);

    assert(tree->root->crt == NULL);
    assert(res == QUAD_FAILED);

    quad_tree_destroy(tree);
    DONE();
}


static void test_quad_tree_insert_replace() {
    DESCRIBE("replace if (n2.pos == n1.pos)");
    QuadTree *tree = quad_tree_create((Vec2){1.f, 1.f}, (Vec2) {10.f, 10.f});

    Creature *crt1 = crt_create(111);
    Creature *crt2 = crt_create(222);
    crt1->pos = (Vec2) {8.f, 2.f};
    crt2->pos = (Vec2) {8.f, 2.f};

    int res;

    {
        // first node
        res = quad_tree_insert(tree, crt1);

        assert(res == QUAD_INSERTED);
        assert(tree->length == 1);
        assert(tree->root->crt != NULL);
        assert(tree->root->crt->id == crt1->id);
    } {
        // second node replaces first
        res = quad_tree_insert(tree, crt2);

        assert(res == QUAD_REPLACED);
        assert(tree->length == 1);
        assert(tree->root->crt != NULL);
        assert(tree->root->crt->id == crt2->id);

        // splitting
        {
            printf("     - no splitting occured\n");
            assert(tree->root->nw == NULL);
            assert(tree->root->ne == NULL);
            assert(tree->root->se == NULL);
            assert(tree->root->sw == NULL);
        }
    }

    quad_tree_destroy(tree);
    DONE();
}

static void test_quad_tree_find() {
    DESCRIBE("find");
    QuadTree *tree = quad_tree_create((Vec2){1.f, 1.f}, (Vec2) {10.f, 10.f});

    Creature *crt1 = crt_create(111);
    Creature *crt2 = crt_create(222);
    crt1->pos = (Vec2) {8.f, 2.f};
    crt2->pos = (Vec2) {1.f, 1.f};

    Creature *crt;
    int res;
    Vec2 search;

    {
        // insert nodes
        res = quad_tree_insert(tree, crt1);
        assert(res == QUAD_INSERTED);

        res = quad_tree_insert(tree, crt2);
        assert(res == QUAD_INSERTED);

        assert(tree->length == 2);
    } {
        // find second non-existing node;
        search = (Vec2){0};
        crt = quad_tree_find(tree, search);
        assert(crt == NULL);
    } {
        // find second item;
        search = crt2->pos;
        crt = quad_tree_find(tree, search);
        assert(crt != NULL);
        assert(crt->id == crt2->id);
    } {
       //find first item
        search = crt1->pos;
        crt = quad_tree_find(tree, search);
        assert(crt != NULL);
        assert(crt->id == crt1->id);
    }

    quad_tree_destroy(tree);
    DONE();
}

void test_qtree(int argc, char **argv) {

    test_tree();
    test_node();
    test_node_bounds();
    test_quad_tree_insert();
    test_quad_tree_insert_outside();
    test_quad_tree_insert_replace();
    test_quad_tree_find();

    return;
}
