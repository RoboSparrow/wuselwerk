#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <math.h>

#include "test.h"
#include "vec2.h"

void test_vec2(int argc, char **argv) {

    GROUP("vec2_norm()");

    {
        DESCRIBE("vector 1,1 should normalize to ~0.7071,0.7071");

        Vec2 v = {1.f, 1.f};

        Vec2 r = vec2_norm(v);
        assert(fabs(r.x - 0.7071) < 0.0001);
        assert(fabs(r.y - 0.7071) < 0.0001);

        DONE();
    }

    {
        DESCRIBE("vector 1,0 should normalize to 1,0");

        Vec2 v = {1.f, 0};
        Vec2 r = vec2_norm(v);
        assert(r.x == 1.f);
        assert(r.y == 0);

        DONE();
    }

    {
        DESCRIBE("vector 2,1 should normalize to ~0.8944,0.44721359");

        Vec2 v = {2.f, 1.f};
        Vec2 r = vec2_norm(v);
        assert(fabs(r.x - 0.8944) < 0.0001);
        assert(fabs(r.y - 0.4472) < 0.0001);

        DONE();
    }

    GROUP("vec2_sub()");

    {
        DESCRIBE("vec2_sub 1,2");

        Vec2 v = {0};
        Vec2 s = {1.f, 2.f};
        Vec2 r = vec2_sub(v, s);
        assert(r.x == -1.f);
        assert(r.y == -2.f);

        DONE();
    }

    {
        DESCRIBE("vec2_sub -1,-2");

        Vec2 v = {0};
        Vec2 s = {-1.f, -2.f};
        Vec2 r = vec2_sub(v, s);
        assert(r.x == 1.f);
        assert(r.y == 2.f);

        DONE();
    }

    GROUP("Vector conversion");

    {
        DESCRIBE("cartesian => polar");

        {
            Vec2 v = {0};
            PVec2 p = vec2_cartesian_to_polar(v);

            assert(p.r == 0.0);
            assert(p.phi == 0.0);
        } {
            Vec2 v = {0.0, 1.0};
            PVec2 p = vec2_cartesian_to_polar(v);

            assert(p.r == 1.0);
            assert(p.phi - M_PI_2 < 0.000001); // 90 deg
        }

        DONE();
    }

    {
        DESCRIBE("polar => cartesian");

        {
            PVec2 p = {0};
            Vec2 v = vec2_polar_to_cartesian(p);

            assert(v.x == 0.0);
            assert(v.y == 0.0);
        } {
            PVec2 p = {1.0, M_PI_2};
            Vec2 v = vec2_polar_to_cartesian(p);

            assert(fabs(v.x) < 0.0001);
            assert(fabs(v.y - 1.0) < 0.0001); // 90 deg
        }

        DONE();
    }

    return;
}
