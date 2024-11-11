#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include "test.h"
#include "qtree.h"

void test_qnode_list(int argc, char **argv) {
    {
        DESCRIBE("qlist_create()");

        size_t sz = 2;
        QuadList *list = qlist_create(sz);
        // qnode_print(tree->root);

        assert(list->len == 0);
        assert(list->max == sz);
        assert(list->grow == sz);
        assert(list->nodes != NULL);

        qlist_destroy(list);
        DONE();
    } {
        DESCRIBE("qlist_append()");

        QuadNode q1 = {
            .center={1.f, 1.f}
        };
        QuadNode q2 = {
            .center={2.f, 2.f}
        };

        QuadList *list = qlist_create(1);
        assert(list->len == 0);
        assert(list->max == 1);
        assert(list->grow == 1);

        qlist_append(list, &q1);
        assert(list->len == 1);
        assert(list->max == 1);
        assert(list->grow == 1);

        qlist_append(list, &q2);
        assert(list->len == 2);
        assert(list->max == 2);
        assert(list->grow == 1);

        qlist_destroy(list);
        DONE();
    } {
        DESCRIBE("qlist_reset()");

        QuadNode q1 = {
            .center={1.f, 1.f}
        };
        QuadNode q2 = {
            .center={2.f, 2.f}
        };

        QuadList *list = qlist_create(1);
        qlist_append(list, &q1);
        qlist_append(list, &q2);

        assert(list->len == 2);
        assert(list->max == 2);
        assert(list->grow == 1);

        qlist_reset(list);

        assert(list->len == 0);
        assert(list->max == 2);
        assert(list->grow == 1);

        assert(list->nodes[0] == NULL);
        assert(list->nodes[1] == NULL);

        qlist_destroy(list);
        DONE();
    } {
        DESCRIBE("qlist_destroy()");

        qlist_destroy(NULL);

        DONE();
    }
}
