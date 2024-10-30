////
// clear && make clean && make && ./critters -f 50 -g 5x5 -c 5 -D
////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <assert.h>
#include <time.h>
#include <string.h>

#include "test.h"


enum Sections {
    TEST_NONE,
    TEST_VEC2,
    TEST_CRT,
    TEST_QTREE,
    TEST_MAX
};

char sections[][256] = {
    "TEST_NONE",
    "TEST_VEC2",
    "TEST_CRT",
    "TEST_QTREE",
    "TEST_MAX"
};

unsigned int scount = 0;
unsigned int gcount = 0;

int selected[TEST_MAX] = { 0 };

void print_test_sections(FILE *fp) {
    if (!fp) {
        fprintf(stderr, "No FILE pointer fp");
        return;
    }
    for(unsigned int i = 1; i <= TEST_MAX; i++) {
        fprintf(fp, "  (%d) %s\n", i, sections[i]);
    }
    return;
}

void input_select(char *optarg)  {
    fprintf(stdout, "\nSelect test sections (comma-separated)\n");
    print_test_sections(stdout);
    fprintf(stdout, "Enter selection: ");

    char in[256];
    fgets(in, sizeof(in), stdin);

    unsigned pos = 0;
    char *part = strtok(in, ",\r\n");

    while(part != NULL){
        selected[pos] = atoi(part);

        if (selected[pos] < 1 || selected[pos] > TEST_MAX) {
            fprintf(stderr, "-invalid selection on position %d: '%s (%d)'\n", pos, part, selected[pos]);
            exit(1);
        }
        pos++;

        part = strtok(NULL, ",\r\n");
    }

    return;
}

int main(int argc, char **argv) {

    srand(time(NULL)); // set random seed
    int section;

    selected[0] = TEST_MAX;

    int opt;
    char usage[] = "usage: %s [-l list sections] [-i interactive] section1 section2..\n";
    while ((opt = getopt(argc, argv, "il")) != -1) {
        switch (opt)  {
            case 'i': {
                input_select(optarg);
            }
            break;
            case 'l': {
                fprintf(stdout, "\nTest sections:\n");
                print_test_sections(stdout);
                fprintf(stdout, "\n");
                exit(0);
            }
            break;
            case 'h':
            case '?':
                fprintf(stderr, usage, argv[0]);
                exit(0);
            break;
        }
    }

    // process the remaining non-option (section numbers)

    unsigned pos = 0;
    for (unsigned int i = optind; i < argc; i++){
        selected[pos] = atoi(argv[i]);
        if (selected[pos] < 1 || selected[pos] > TEST_MAX) {
            fprintf(stderr, "-invalid selection on position %d: '%s (%d)'\n", pos, argv[i], selected[pos]);
            exit(1);
        }
        pos++;
    }

    // run tests

    for (unsigned int i = 0; i < TEST_MAX; i++) {
        section = selected[i];
        //printf(" * %d, %d, %s\n", i, section, sections[section]);
        if (section <= 0) {
            break;
        }

        if (section == TEST_VEC2 || section == TEST_MAX) {
            // test.vec2.c
            SECTION(sections[TEST_VEC2]);
            test_vec2(argc, argv);
        }

        if (section == TEST_CRT || section == TEST_MAX) {
            // test.vec2.c
            SECTION(sections[TEST_CRT]);
            test_crt(argc, argv);
        }

        if (section == TEST_QTREE || section == TEST_MAX) {
            // test.vec2.c
            SECTION(sections[TEST_QTREE]);
            test_qtree(argc, argv);
        }

    }

    fprintf(stderr,
        "\n--------------------\n"
        "-- \033[01;32mTESTS FINISHED\033[0m --"
        "\n--------------------\n\n"
        "stats: %d sections, %d groups\n\n",
        scount, gcount
    );
    return 0;
}
