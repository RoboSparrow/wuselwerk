#ifndef __POP_H__
#define __POP_H__

#include "crt.h"

#define POP_MAX 100

typedef struct Creature Creature; // forward declaration (TODO, why neccessary here?)

typedef struct Population {
    size_t len;
    Creature *members[POP_MAX];
} Population;

void pop_destroy(Population *pop);
void pop_print(FILE *fp, Population *pop);

#endif
