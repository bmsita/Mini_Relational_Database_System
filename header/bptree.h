//header/bptree.h
#ifndef BPTREE_H
#define BPTREE_H

#include <stdbool.h>
#include "record.h"

#define BP_ORDER 3

typedef struct BPTreeNode {
    bool is_leaf;
    int num_keys;
    int keys[BP_ORDER];
    void *children[BP_ORDER + 1];
    struct BPTreeNode *next;
} BPTreeNode;

typedef struct {
    BPTreeNode *root;
    int order;
} BPTree;

void bptree_init(BPTree *tree);
void bptree_insert(BPTree *tree, int key, Record *rec);
void bptree_delete(BPTree *tree, int key);
Record *bptree_search(BPTree *tree, int key);
void bptree_traverse(BPTree *tree); 
void bptree_print(BPTree *tree);
void bptree_free(BPTreeNode *node);
void bptree_for_each(BPTree *tree, void (*fn)(Record *r, void *ctx), void *ctx);

#endif