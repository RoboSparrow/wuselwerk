#include <stdio.h>
#include <stdlib.h>

#include "crt.h"
#include "pop.h"

void pop_destroy(Population *pop) {
    for (int i = 0; i < pop->len; i++) {
        crt_destroy(pop->members[i]);
    }
    pop->len = 0;
}

void pop_print(FILE *fp, Population *pop) {
    if(!fp) {
        return;
    }
    if (!pop) {
        fprintf(fp, "<NULL>\n");
    }

    fprintf(fp, "{ len: %ld, members: [", pop->len);
    for (int i = 0; i < pop->len; i++) {
        fprintf(fp, "{%d, \"%s\", %s}%s", pop->members[i]->id, pop->members[i]->name, CRT_TYPE_NAME(pop->members[i]->type), (i < pop->len - 1) ? ", " : "");
    }
    fprintf(fp, "] }\n");
}
