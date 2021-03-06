#include "huffman.h"

#include "code.h"
#include "defines.h"
#include "node.h"
#include "pq.h"
#include "stack.h"

#include <stdio.h>

// Build the tree
Node *build_tree(uint64_t hist[static ALPHABET]) {
    Node *n;
    Node *left;
    Node *right;
    Node *joined = NULL;
    PriorityQueue *pq = pq_create(ALPHABET);

    // Loop thorugh alphabet
    for (int i = 0; i < ALPHABET; i++) {
        // If there is a histogram entry for a character create a node for the character and equeue
        if (hist[i] != 0) {
            n = node_create(i, hist[i]);
            enqueue(pq, n);
        }
    }

    // Dequeue the 2 nodes (left and right) and enqueue the joined node
    // The last node left is the root node
    while (pq_size(pq) > 1) {
        dequeue(pq, &left);
        dequeue(pq, &right);
        joined = node_join(left, right);
        enqueue(pq, joined);
    }

    dequeue(pq, &joined);
    pq_delete(&pq);
    return joined;
}

// Build codes given a root node and a table of codes
void build_codes(Node *root, Code table[static ALPHABET]) {
    static Code c = { 0, { 0 } };
    uint8_t bit;
    // If the root exists
    if (root) {

        // At a leaf node set the index of symbol to the code
        if (root->left == NULL && root->right == NULL) {
            table[root->symbol] = c;
        } else {
            // Recursively call with left. Left is represented by 0 and right is represetned by 1
            code_push_bit(&c, 0);
            build_codes(root->left, table);
            code_pop_bit(&c, &bit);

            code_push_bit(&c, 1);
            build_codes(root->right, table);
            code_pop_bit(&c, &bit);
        }
    }
    return;
}

Node *rebuild_tree(uint16_t nbytes, uint8_t tree_dump[static nbytes]) {
    Node *n;
    Node *right;
    Node *left;
    Stack *s = stack_create(ALPHABET);
    for (int i = 0; i < nbytes; i++) {
        if (tree_dump[i] == 'L') {
            i = i + 1;
            n = node_create(tree_dump[i], 0);
            stack_push(s, n);
        } else {
            stack_pop(s, &right);
            stack_pop(s, &left);
            stack_push(s, node_join(left, right));
        }
    }
    stack_pop(s, &n);
    stack_delete(&s);
    return n;
}

void delete_tree(Node **root) {
    if (*root) {
        delete_tree(&(*root)->left);
        delete_tree(&(*root)->right);
        node_delete(root);
    }
    return;
}
