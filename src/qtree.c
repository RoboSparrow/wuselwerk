#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <assert.h>

#include "qtree.h"
#include "utils.h"

////
// QuadNode
////

// forward declarations
static int _node_split(QuadTree *tree, QuadNode *node);
void qnode_print(FILE *fp, QuadNode *node);

/**
 * Checks if a pos is with an node boundary.
 */
static int _node_contains(QuadNode *node, Vec2 pos) {
    // printf("-- nw: {%f, %f}, se: {%f, %f}, crt: {%f, %f}\n", node->self_nw.x, node->self_nw.y, node->self_se.x, node->self_se.y, pos.x, pos.y);
    return node != NULL
        && node->self_nw.x <= pos.x
        && node->self_nw.y <= pos.y
        && node->self_se.x >= pos.x
        && node->self_se.y >= pos.y; // TODO quadtree.c rules are different (using bounds.nw, se as min and max)
}

/**
 * Gets the matching quadrant child node for a given position.
 */
static QuadNode *_node_quadrant(QuadNode *node, Vec2 pos) {
    // printf("-- nw: {%f, %f}, se: {%f, %f}, crt: {%f, %f}\n", node->self_nw.x, node->self_nw.y, node->self_se.x, node->self_se.y, pos.x, pos.y);
    if(_node_contains(node->nw, pos)) {
        return node->nw;
    }
    if(_node_contains(node->ne, pos)) {
        return node->ne;
    }
    if(_node_contains(node->sw, pos)) {
        return node->sw;
    }
    if(_node_contains(node->se, pos)) {
        return node->se;
    }
    return NULL;
}

/**
 * Replaces an entity node.
 */
static void _node_reset(QuadTree *tree, QuadNode *node) {
    crt_destroy(node->crt);
    node->crt = NULL;
}

/**
 * Inserts an entity into a tree node. The node might be split into four childs, or the  already existing entity in this node might be replaced
 * Note: The position bounds must be checked by callee (qtree_insert())
 */
static int _node_insert(QuadTree *tree, QuadNode *node, Creature *crt) {
    if (!tree || !crt) { // !node?
        return QUAD_FAILED;
    }

    // 1. insert into THIS (empty) node (just created before)
    if (qnode_isempty(node)) {
        node->crt = crt;
        return QUAD_INSERTED;
    }

    // 2. replace THIS node OR split and insert into CHILDREN
    if (qnode_isleaf(node)) {
        // 2.1 pos match: replace
        if (node->crt->pos.x == crt->pos.x && node->crt->pos.y == crt->pos.y) {
            _node_reset(tree, node);
            node->crt = crt;
            return QUAD_REPLACED;
        }

        // 2.2 split node (and also mv previous node)
        if (_node_split(tree, node) == QUAD_FAILED) {
            return QUAD_FAILED;
        }

        // 2.3. insertcurrent node
        return _node_insert(tree, node, crt);
    }

    // 3. insert into one of THIS CHILDREN
    if (qnode_ispointer(node)) {
        QuadNode *child = _node_quadrant(node, crt->pos);
        if(!child) {
             return QUAD_FAILED;
        }
        return _node_insert(tree, child, crt);
    }

    return QUAD_FAILED;
}

/**
 * Spits a quadrant nodes into 4 child quadrants.
 * Moves a existing entity node into the matching quadrant.
 */
static int _node_split(QuadTree *tree, QuadNode *node) {
    if (!tree || !node) {
        return QUAD_FAILED;
    }

    QuadNode *nw = qnode_create(node);
    QuadNode *ne = qnode_create(node);
    QuadNode *sw = qnode_create(node);
    QuadNode *se = qnode_create(node);

    if (!nw || !ne || !sw || !se) {
        return QUAD_FAILED;
    }

    Creature *old = node->crt;

    // nw(x,y)            hw
    // x────────────┬────────────┐
    // │            │            │
    // │            │            │
    // │     nw     │     ne     │ hh
    // │            │            │
    // │            │            │
    // ├────────────x────────────┤
    // │            │ c(x,y)     │
    // │            │            │
    // │     sw     │     se     │
    // │            │            │
    // │            │            │
    // └────────────┴────────────x
    //                     se(x,y)

    float hw = node->self_width / 2;
    float hh = node->self_height / 2;

    float minx = node->self_nw.x;
    float miny = node->self_nw.y;
    float ctrx = minx + hw;
    float ctry = miny + hh;
    float maxx = node->self_se.x;
    float maxy = node->self_se.y;

    /*                         nw                     se              */
    qnode_set_bounds(nw,  (Vec2) {minx, miny}, (Vec2) {ctrx, ctry});
    qnode_set_bounds(ne,  (Vec2) {ctrx, miny}, (Vec2) {maxx, ctry});
    qnode_set_bounds(se,  (Vec2) {ctrx, ctry}, (Vec2) {maxx, maxy});
    qnode_set_bounds(sw,  (Vec2) {minx, ctry}, (Vec2) {ctrx, maxy});

    node->nw = nw;
    node->ne = ne;
    node->sw = sw;
    node->se = se;

    node->crt = NULL;

    return _node_insert(tree, node, old); // inserts into one of the children
}

/**
 * Find a node for a given position
 * uses same conditions as _node_find()
 */
QuadNode *_node_find(QuadTree *tree, QuadNode *node, Vec2 pos) {
    if (!tree || !node) {
        return NULL;
    }

    if(qnode_isleaf(node)) {
        if (node->crt->pos.x == pos.x && node->crt->pos.y == pos.y) {
            return node;
        }
    }

    if (qnode_ispointer(node)) {
        QuadNode *child = _node_quadrant(node, pos);
        if(!child) {
             return NULL;
        }
        return _node_find(tree, child, pos);
    }

    return NULL;
}

// --- public

QuadNode *qnode_create(QuadNode *parent) {
    QuadNode *node = malloc(sizeof(QuadNode));
    if (!node) {
        LOG_ERROR("failed to allaocate memeory for QuadNode");
        return NULL;
    }

    node->parent = parent;

    node->ne = NULL;
    node->nw = NULL;
    node->se = NULL;
    node->sw = NULL;

    node->self_nw  = (Vec2){0};
    node->self_se  = (Vec2){0};

    node->self_width  = 0;
    node->self_height  = 0;

    node->crt = NULL;

    return node;
}

void qnode_destroy(QuadNode *node) {
    if (!node) {
        return;
    }
    if (node->nw) {
        qnode_destroy(node->nw);
    }
    if (node->ne) {
        qnode_destroy(node->ne);
    }
    if (node->sw) {
        qnode_destroy(node->sw);
    }
    if (node->se) {
        qnode_destroy(node->se);
    }

    // !!!! We  don not manage the memory of the item pointer here !!!!
    // crt_destroy(node->crt);

    node->crt = NULL;

    freez(node);
}

int qnode_isleaf(QuadNode *node) {
    return node->crt != NULL;
}

// TODO rename
int qnode_ispointer(QuadNode *node) {
  return node->nw != NULL
      && node->ne != NULL
      && node->sw != NULL
      && node->se != NULL
      && !qnode_isleaf(node);
}

int qnode_isempty(QuadNode *node) {
  return node->nw == NULL
      && node->ne == NULL
      && node->sw == NULL
      && node->se == NULL
      && !qnode_isleaf(node);
}

void qnode_set_bounds(QuadNode *node, Vec2 nw, Vec2 se) {
    node->self_nw =  nw;
    node->self_se =  se;

    node->self_width  = fabs(nw.x - se.x);
    node->self_height = fabs(nw.y - se.y);
}

/**
 * recusively walk trough a quad node's children and apply on before (recusing) and on after call backs
 */
void qnode_walk(QuadNode *node, void (*descent)(QuadNode *node), void (*ascent)(QuadNode *node)) {
  (*descent)(node);

  if(node->nw != NULL) {
      qnode_walk(node->nw, descent, ascent);
  }
  if(node->ne != NULL) {
      qnode_walk(node->ne, descent, ascent);
  }
  if(node->sw != NULL) {
      qnode_walk(node->sw, descent, ascent);
  }
  if(node->se != NULL) {
      qnode_walk(node->se, descent, ascent);
  }

  (*ascent)(node);
}

////
// QuadTree
////

QuadTree *qtree_create(Vec2 window_nw, Vec2 window_se) {
    assert(window_nw.x < window_se.x);
    assert(window_nw.y < window_se.y);

    QuadTree *tree = malloc(sizeof(QuadTree));
    if (!tree) {
        return NULL;
    }

    tree->root = qnode_create(NULL);
    if (!tree->root) {
        return NULL;
    }

    qnode_set_bounds(tree->root, window_nw, window_se);
    tree->length = 0;

    return tree;
}

void qtree_destroy(QuadTree *tree) {
    if (!tree) {
        return;
    }
    qnode_destroy(tree->root);
    freez(tree);
}

int qtree_insert(QuadTree *tree, Creature *crt) {
    if (!tree || !crt) {
        return QUAD_FAILED;
    }

    // check if pos is in tree bounds
    if(!_node_contains(tree->root, crt->pos)) {
        return QUAD_FAILED;
    }

    int status = _node_insert(tree, tree->root, crt);
    if (status == QUAD_INSERTED) {
        tree->length++;
    }

    return status;
}

QuadNode *qtree_find(QuadTree *tree, Vec2 pos) {
    if (!tree) {
        return NULL;
    }
    return _node_find(tree, tree->root, pos);
}

////
// debug
////

FILE *_out = NULL;

void _print_desc(QuadNode *node) {
    qnode_print(_out, node);
}

void _print_asc(QuadNode *node) {
  fprintf(_out, "\n");
}

// --- public

void qtree_print(FILE *fp, QuadTree *tree) {
    if (!tree) {
        fprintf(fp, "<NULL>");
        return;
    }
    _out = fp;
    qnode_walk(tree->root, _print_asc, _print_desc);
    printf("\n");
    _out = NULL;
}

void qnode_print(FILE *fp, QuadNode *node) {
    if (!node) {
        fprintf(fp, "<NULL>");
        return;
    }
    fprintf(fp, "{self_nw: {%f, %f}, self_se: {%f, %f}, ", node->self_nw.x, node->self_nw.y, node->self_se.x, node->self_se.y);
    fprintf(fp, "parent: '%c', ", (node->parent) ? 'y' : '-');
    fprintf(fp, "nw: '%c', sw: '%c', se: '%c', nw: '%c', ", (node->nw) ? 'y' : '-', (node->sw) ? 'y' : '-', (node->se) ? 'y' : '-', (node->ne) ? 'y' : '-');
    if (node->crt) {
        fprintf(fp, "crt: {id: %d, x: %f, y: %f}}", node->crt->id, node->crt->pos.x, node->crt->pos.y);
    } else {
        fprintf(fp, "crt: <NULL>}");
    }
}


////
// area search
////

static int _node_within_area(QuadNode *node, Vec2 nw, Vec2 se) {
    return node != NULL
        && node->self_nw.x >= nw.x
        && node->self_se.x <= se.x
        && node->self_nw.y >= nw.y
        && node->self_se.y <= se.y;
}

static int _node_overlaps_area(QuadNode *node, Vec2 nw, Vec2 se) {
    return node != NULL
        && node->self_nw.x < se.x
        && node->self_se.x >= nw.x
        && node->self_nw.y < se.y
        && node->self_se.y >= nw.y;
}

static void _node_find_in_area(QuadNode *node, Creature *crt, Vec2 nw, Vec2 se, CrtList *list) {
    if (!node || !crt) {
        return;
    }

    // this node does not interesect with the search boundary
    // stop searching this branch
    if (!_node_overlaps_area(node, nw, se)) {
        return;
    }
    // todo if contains add all chldren withaout checking

    if(qnode_isleaf(node)) {
        // is not the home node or crt
        if (node->crt->id == crt->id) {
            return;
        }
        // If this node is within the search boundary
        if(_node_within_area(node, nw, se)) {
            crt_list_append(list, node->crt);
        }

        return;
    }

    if (node->nw) {
        _node_find_in_area(node->nw, crt, nw, se, list);
    }
    if (node->ne) {
        _node_find_in_area(node->ne, crt, nw, se, list);
    }
    if (node->se) {
        _node_find_in_area(node->se, crt, nw, se, list);
    }
    if (node->sw) {
        _node_find_in_area(node->sw, crt, nw, se, list);
    }

}

CrtList *qtree_find_in_area(QuadTree *tree, Creature *crt, Vec2 nw, Vec2 se) {
    if (!tree || !crt) {
        return NULL;
    }

    CrtList *list = crt_list_create(5);
    EXIT_IF(list == NULL, "failed to allocate memory for CrtList");

    _node_find_in_area(tree->root, crt, nw, se, list);
    return list;
}
