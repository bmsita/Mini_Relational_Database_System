//src/bptree.c
#include "bptree.h"
#include "record.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static BPTreeNode *bptree_create_node(bool is_leaf) {
    BPTreeNode *node = (BPTreeNode*)malloc(sizeof(BPTreeNode));
    node->is_leaf = is_leaf;
    node->num_keys = 0;
    node->next = NULL;
    for (int i = 0; i <= BP_ORDER; i++)
        node->children[i] = NULL;
    return node;
}

void bptree_init(BPTree *tree) {
    tree->root = bptree_create_node(true);
    tree->order = BP_ORDER;
    printf("[B+Tree] Initialized (order=%d)\n", BP_ORDER);
}


static void bptree_insert_into_leaf(BPTreeNode *leaf, int key, Record *rec) {
    int i = leaf->num_keys - 1;
    while (i >= 0 && leaf->keys[i] > key) {
        leaf->keys[i + 1] = leaf->keys[i];
        leaf->children[i + 1] = leaf->children[i];
        i--;
    }
    leaf->keys[i + 1] = key;
    leaf->children[i + 1] = rec;
    leaf->num_keys++;
}

static BPTreeNode *bptree_split_leaf(BPTreeNode *leaf) {
    int split = leaf->num_keys / 2;
    BPTreeNode *new_leaf = bptree_create_node(true);
    new_leaf->num_keys = leaf->num_keys - split;

    for (int i = 0; i < new_leaf->num_keys; i++) {
        new_leaf->keys[i] = leaf->keys[split + i];
        new_leaf->children[i] = leaf->children[split + i];
    }

    leaf->num_keys = split;
    new_leaf->next = leaf->next;
    leaf->next = new_leaf;
    return new_leaf;
}

void bptree_insert(BPTree *tree, int key, Record *rec) {
    if (!tree->root) {
        tree->root = bptree_create_node(true);
    }

    BPTreeNode *root = tree->root;

    if (root->num_keys == tree->order) {
        BPTreeNode *new_root = bptree_create_node(false);
        new_root->children[0] = root;
        BPTreeNode *split = bptree_split_leaf(root);

        new_root->keys[0] = split->keys[0];
        new_root->children[1] = split;
        new_root->num_keys = 1;

        tree->root = new_root;
        printf("[B+Tree] Root split occurred\n");
    }

    BPTreeNode *node = tree->root;
    while (!node->is_leaf) {
        int i = 0;
        while (i < node->num_keys && key >= node->keys[i]) i++;
        node = (BPTreeNode*)node->children[i];
    }

    bptree_insert_into_leaf(node, key, rec);

    if (node->num_keys > tree->order) {
        BPTreeNode *split = bptree_split_leaf(node);
        printf("[B+Tree] Leaf split: new leaf starts with key=%d\n", split->keys[0]);
    }
    printf("[B+Tree] Inserted key=%d (record name=%s)\n", key, rec->name);
}

Record *bptree_search(BPTree *tree, int key) {
    BPTreeNode *node = tree->root;
    while (node && !node->is_leaf) {
        int i = 0;
        while (i < node->num_keys && key >= node->keys[i]) i++;
        node = (BPTreeNode*)node->children[i];
    }

    if (!node) return NULL;

    for (int i = 0; i < node->num_keys; i++)
        if (node->keys[i] == key)
            return (Record*)node->children[i];

    return NULL;
}

void bptree_delete(BPTree *tree, int key) {
    BPTreeNode *node = tree->root;
    while (node && !node->is_leaf) {
        int i = 0;
        while (i < node->num_keys && key >= node->keys[i]) i++;
        node = (BPTreeNode *)node->children[i];
    }

    if (!node) return;

    for (int i = 0; i < node->num_keys; i++) {
        if (node->keys[i] == key) {
            Record *r = (Record *)node->children[i];
            if (r) free(r);
            for (int j = i; j < node->num_keys - 1; j++) {
                node->keys[j] = node->keys[j + 1];
                node->children[j] = node->children[j + 1];
            }
            node->num_keys--;
            printf("[B+Tree] Deleted key=%d\n", key);
            return;
        }
    }

    printf("[B+Tree] Key=%d not found for delete.\n", key);
}


void bptree_traverse(BPTree *tree) {
    BPTreeNode *cur = tree->root;
    if (!cur) {
        printf("(no records)\n");
        return;
    }

    while (!cur->is_leaf)
        cur = cur->children[0];

    while (cur) {
        for (int i = 0; i < cur->num_keys; i++) {
            Record *r = (Record *)cur->children[i];
            printf("%d | %-10s | %d\n", r->id, r->name, r->marks);
        }
        cur = cur->next;
    }
}
void bptree_print(BPTree *tree) {
    printf("[B+Tree] Print (root has %d keys)\n", tree->root ? tree->root->num_keys : 0);
    bptree_traverse(tree);
}

void bptree_for_each(BPTree *tree, void (*fn)(Record *r, void *ctx), void *ctx) {
    if (!tree || !tree->root || !fn) return;
    BPTreeNode *cur = tree->root;
    while (!cur->is_leaf) {
        cur = (BPTreeNode*)cur->children[0];
        if (!cur) return;
    }
    while (cur) {
        for (int i = 0; i < cur->num_keys; ++i) {
            Record *r = (Record*)cur->children[i];
            if (r) fn(r, ctx);
        }
        cur = cur->next;
    }
}

void bptree_free(BPTreeNode *node){
    if (!node) return;
    if (node->is_leaf) {
        for (int i = 0; i < node->num_keys; ++i) {
            Record *r = (Record*)node->children[i];
            if (r) free(r);
        }
    } else {
        for (int i = 0; i <= node->num_keys; i++) {
            bptree_free((BPTreeNode*)node->children[i]);
        }
    }
    free(node);
}