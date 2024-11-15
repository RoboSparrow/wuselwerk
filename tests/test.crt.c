#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <math.h>

#include "test.h"
#include "crt.h"

void test_crt(int argc, char **argv) {

    GROUP("Creature creation");

    {
        DESCRIBE("CRT_INIT()");

        Creature c = CRT_INIT(123);

        // lazy tesing some props
        assert(c.id == 123);
        assert(c.pos.x == CRT_POS_NONE);
        assert(c.targ.y == CRT_POS_NONE);

        DONE();
    }

    {
        DESCRIBE("crt_create()");

        Creature *c = crt_create(1234);

        // strict testing all props
        assert(c->id == 1234);
        assert(strlen(c->name) == 0);
        assert(c->type == CRT_TYPE_NONE);
        assert(c->status == CRT_STATUS_NONE);
        assert(c->agility == 0.0);
        assert(c->size == 0.0);
        assert(c->pos.x == CRT_POS_NONE);
        assert(c->pos.y == CRT_POS_NONE);
        assert(c->targ.x == CRT_POS_NONE);
        assert(c->targ.y == CRT_POS_NONE);

        crt_destroy(c);

        DONE();
    }

    {
        DESCRIBE("crt_birth()");

        Creature *c = crt_birth(12345, "c12345", CRT_TYPE_HERBIVORE, (Vec2){1.0, 2.0});

        // strict testing all props
        assert(c->id == 12345);
        assert(strncmp(c->name, "c12345", CRT_NAME_LEN) == 0);
        assert(c->type == CRT_TYPE_HERBIVORE);
        assert(c->status == CRT_STATUS_ALIVE);
        assert(c->agility == CRT_MIN_AGILITY);
        assert(c->size == CRT_MIN_SIZE);
        assert(c->pos.x == 1.0);
        assert(c->pos.y == 2.0);
        assert(c->targ.x == 1.0);
        assert(c->targ.y == 2.0);

        crt_destroy(c);

        DONE();
    }
}
