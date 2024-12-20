#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include "test.h"
#include "qtree.h"

typedef struct TestItem {
    int id;
    Vec2 pos; // control data, will not be queried within qtree.h
} TestItem;

static void test_tree() {
    DESCRIBE("tree");
    QuadTree *tree;
    QuadNode *root;

    tree = qtree_create((Vec2) {0}, (Vec2) {600.f, 400.f});
    assert(tree->length == 0);

    root = tree->root;
    assert(!qnode_isleaf(root));
    assert(qnode_isempty(root));
    assert(!qnode_ispointer(root));

    assert(root->self_nw.x == 0);
    assert(root->self_nw.y == 0);
    assert(root->self_se.x == 600.0);
    assert(root->self_se.y == 400.0);

    assert(root->width == 600.0);
    assert(root->height == 400.0);

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

    assert(node->width == 1.0);
    assert(node->height == 2.0);

    qnode_destroy(node);
    DONE();
}

static void test_tree_insert() {
    QuadTree *tree =  qtree_create((Vec2) {1.f, 1.f}, (Vec2) {10.f, 10.f});
    assert(tree != NULL);
    assert(tree->root->self_nw.x == 1.0);
    assert(tree->root->self_nw.y == 1.0);
    assert(tree->root->self_se.x == 10.0);
    assert(tree->root->self_se.y == 10.0);

    TestItem itm1 = {111, {8.f, 2.f}};
    TestItem itm2 = {222, {1.f, 1.f}};

    {
        DESCRIBE("test_qtree_insert(first node)");
        int res = qtree_insert(tree, &itm1, itm1.pos);
        // qnode_print(tree->root);

        assert(tree->root->data != NULL);
        assert(res == QUAD_INSERTED);
        assert(tree->length == 1);

        assert(tree->root->nw == NULL);
        assert(tree->root->ne == NULL);
        assert(tree->root->se == NULL);
        assert(tree->root->sw == NULL);

        // verify node idendity
        assert(tree->root->data != NULL);
        assert(tree->root->pos.x == itm1.pos.x);
        assert(tree->root->pos.y == itm1.pos.y);

        TestItem *item = (TestItem*) tree->root->data;
        assert(item->id == itm1.id);
        DONE();
    } {
        DESCRIBE("test_qtree_insert(second node)");
        int res = qtree_insert(tree, &itm2, itm2.pos);
        // qnode_print(tree->root);

        assert(tree->root->data == NULL); // 111 has been moved
        assert(res == QUAD_INSERTED);
        assert(tree->length == 2);

        // splitting
        assert(tree->root->nw != NULL);
        assert(tree->root->ne != NULL);
        assert(tree->root->se != NULL);
        assert(tree->root->sw != NULL);

        // verify node idendity
        assert(tree->root->ne->data != NULL);
        assert(tree->root->ne->pos.x == itm1.pos.x);
        assert(tree->root->ne->pos.y == itm1.pos.y);

        TestItem *item = (TestItem*) tree->root->ne->data;
        assert(item->id == itm1.id);
        DONE();
    }

    qtree_destroy(tree);
}

static void test_tree_insert_outside() {
    DESCRIBE("pos outside of tree");
    QuadTree *tree = qtree_create((Vec2) {1.f, 1.f}, (Vec2) {10.f, 10.f});

    TestItem itm = {111, {0.f, 0.f}};

    int res;
    res = qtree_insert(tree, &itm, itm.pos);

    assert(tree->root->data == NULL);
    assert(tree->root->pos.x == INFINITY);
    assert(tree->root->pos.y == INFINITY);
    assert(res == QUAD_FAILED);

    qtree_destroy(tree);
    DONE();
}


static void test_tree_insert_replace() {
    DESCRIBE("replace if (n2.pos == n1.pos)");
    QuadTree *tree = qtree_create((Vec2) {1.f, 1.f}, (Vec2) {10.f, 10.f});

    TestItem itm1 = {111, {8.f, 2.f}};
    TestItem itm2 = {222, {8.f, 2.f}};

    int res;
    TestItem *item;

    {
        // first node
        res = qtree_insert(tree, &itm1, itm1.pos);

        assert(res == QUAD_INSERTED);
        assert(tree->length == 1);

        assert(tree->root->data != NULL);
        assert(tree->root->pos.x == itm1.pos.x);
        assert(tree->root->pos.y == itm1.pos.y);

        item = (TestItem*) tree->root->data;
        assert(item->id == itm1.id);
    } {
        // second node replaces first
        res = qtree_insert(tree, &itm2, itm2.pos);

        assert(res == QUAD_REPLACED);
        assert(tree->length == 1);

        assert(tree->root->data != NULL);
        assert(tree->root->pos.x == itm2.pos.x);
        assert(tree->root->pos.y == itm2.pos.y);

        item = (TestItem*) tree->root->data;
        assert(item->id == itm2.id);

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
    QuadTree *tree = qtree_create((Vec2) {1.f, 1.f}, (Vec2) {10.f, 10.f});

    TestItem itm1 = {111, {8.f, 2.f}};
    TestItem itm2 = {222, {1.f, 1.f}};

    QuadNode *node;
    TestItem *item;
    int res;
    Vec2 search;

    {
        // insert nodes
        res = qtree_insert(tree, &itm1, itm1.pos);
        assert(res == QUAD_INSERTED);

        res = qtree_insert(tree, &itm2, itm2.pos);
        assert(res == QUAD_INSERTED);

        assert(tree->length == 2);
    } {
        // find second non-existing node;
        search = (Vec2){0};
        node = qtree_find(tree, search);
        assert(node == NULL);
    } {
        // find second item;
        search = itm2.pos;
        node = qtree_find(tree, search);
        assert(node != NULL);

        item = (TestItem*) node->data;
        assert(item->id == itm2.id);
    } {
       //find first item
        search = itm1.pos;
        node = qtree_find(tree, search);

        item = (TestItem*) node->data;
        assert(item->id == itm1.id);
    }

    qtree_destroy(tree);
    DONE();
}

static void test_node_parent() {
    DESCRIBE("parent");
    QuadTree *tree = qtree_create((Vec2) {1.f, 1.f}, (Vec2) {10.f, 10.f});

    // a nested tree with three levels
    TestItem itm1 = {111, {8.f, 2.f}};
    TestItem itm2 = {222, {9.f, 1.f}};

    QuadNode *node, *parent;
    TestItem *item;
    int res;
    Vec2 search;

    {
        // insert nodes
        res = qtree_insert(tree, &itm1, itm1.pos);
        assert(res == QUAD_INSERTED);

        res = qtree_insert(tree, &itm2, itm2.pos);
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

        // qnode_print(stderr, node);
    } {
        // find first leaf
        parent = tree->root->ne->ne;
        node   = tree->root->ne->ne->nw;

        assert(node != NULL);
        assert(node->parent != NULL);

        assert(node->parent->self_nw.x == parent->self_nw.x);
        assert(node->parent->self_se.y == parent->self_se.y);

        // data
        assert(node->data != NULL);

        item = (TestItem*) node->data;
        assert(item->id == itm1.id);

        // qnode_print(stderr, node);
    } {
        // find second leaf
        parent = tree->root->ne->ne;
        node   = tree->root->ne->ne->ne;

        assert(node != NULL);
        assert(node->parent != NULL);

        assert(node->parent->self_nw.x == parent->self_nw.x);
        assert(node->parent->self_se.y == parent->self_se.y);

        // data
        assert(node->data != NULL);

        item = (TestItem*) node->data;
        assert(item->id == itm2.id);

        // qnode_print(stderr, node);
    }

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
}
