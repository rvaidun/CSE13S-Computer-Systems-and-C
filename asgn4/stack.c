#include "stack.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
// Code from assignment PDF
struct Stack {
    uint32_t top;
    uint32_t capacity;
    int64_t *items;
};

// Code from assignment PDF for assignment 3
Stack *stack_create(uint32_t capacity) {
    Stack *s = (Stack *) malloc(sizeof(Stack));
    if (s) {
        s->top = 0;
        s->capacity = capacity;
        s->items = (int64_t *) calloc(capacity, sizeof(int64_t));
        if (!s->items) {
            free(s);
            s = NULL;
        }
    }
    return s;
}

// Code from assignment PDF for assignment 3
void stack_delete(Stack **s) {
    if (*s && (*s)->items) {
        free((*s)->items);
        free(*s);
        *s = NULL;
    }
    return;
}

// Returns true if stack is empty
bool stack_empty(Stack *s) {
    return s->top == 0;
}

// Returns true if stack is full
bool stack_full(Stack *s) {
    return s->top == s->capacity;
}

// Returns the number of items in the stack
uint32_t stack_size(Stack *s) {
    return s->top;
}

// Adds an item to the stack
bool stack_push(Stack *s, uint32_t x) {
    if (!stack_full(s)) {
        s->items[s->top] = x;
        s->top++;
        return true;
    }
    return false;
}

// Peek into a stack to return whats at the top and return through pointer x
bool stack_peek(Stack *s, uint32_t *x) {
    if (!stack_empty(s)) {
        *x = s->items[s->top - 1];
        return true;
    }
    return false;
}

// Removes an item from the stack and returns the item through the pointer x
bool stack_pop(Stack *s, uint32_t *x) {
    if (!stack_empty(s)) {
        s->top--;
        *x = s->items[s->top];
        s->items[s->top] = 0;
        return true;
    }
    return false;
}

// Creates a copy of a stack src to stack destination
void stack_copy(Stack *dst, Stack *src) {
    // Loop through src items and for each item set destination item
    for (uint32_t i = 0; i < src->capacity; i++) {
        dst->items[i] = src->items[i];
    }
    dst->top = src->top;
    return;
}

// Prints a stack
// Code from Assignment PDF
void stack_print(Stack *s, FILE *outfile, char *cities[]) {
    for (uint32_t i = 0; i < s->top; i += 1) {
        fprintf(outfile, "%s", cities[s->items[i]]);
        if (i + 1 != s->top) {
            fprintf(outfile, " -> ");
        }
    }
    fprintf(outfile, "\n");
    return;
}
