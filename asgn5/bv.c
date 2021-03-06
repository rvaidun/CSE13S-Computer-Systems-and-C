#include "bv.h"

#include <stdio.h>
#include <stdlib.h>
struct BitVector {
    uint32_t length;
    uint8_t *vector;
};

// Creates a bit vector
BitVector *bv_create(uint32_t length) {
    BitVector *bv = (BitVector *) malloc(sizeof(BitVector));
    if (bv) {
        bv->length = length;
        if (length % 8 == 0) {
            bv->vector = (uint8_t *) calloc(bv->length / 8, sizeof(uint8_t));
        } else {
            bv->vector = (uint8_t *) calloc((bv->length / 8) + 1, sizeof(uint8_t));
        }
    }
    return bv;
}

// deletes bit vector and frees up any space that has been allocated
void bv_delete(BitVector **v) {
    if (*v && (*v)->vector) {
        free((*v)->vector);
        free(*v);
        *v = NULL;
    }
    return;
}

// returns length of the bit vector
uint32_t bv_length(BitVector *v) {
    return v->length;
}

// set a bit in the bit vector
void bv_set_bit(BitVector *v, uint32_t i) {
    uint32_t bytepos = i / 8;
    uint32_t bitpos = i % 8;
    v->vector[bytepos] |= (1 << bitpos);
    return;
}

// clear a bit from the bit vector
void bv_clr_bit(BitVector *v, uint32_t i) {
    uint32_t bytepos = i / 8;
    uint32_t bitpos = i % 8;
    v->vector[bytepos] &= ~(1 << bitpos);
    return;
}

// get a bit from the bit vector
uint8_t bv_get_bit(BitVector *v, uint32_t i) {
    uint32_t bytepos = i / 8;
    uint32_t bitpos = i % 8;
    return (v->vector[bytepos] >> bitpos) & 1;
}

// xor a bit in the vector with a given bit
void bv_xor_bit(BitVector *v, uint32_t i, uint8_t bit) {
    uint32_t bytepos = i / 8;
    uint32_t bitpos = i % 8;
    uint8_t b = bv_get_bit(v, i);
    v->vector[bytepos] = (v->vector[bytepos] & (~(1 << bitpos)))
                         | ((b ^ bit) << bitpos); // Clear the bit and set it to b
    return;
}

// Gets a bit from the bit vector
uint8_t bv_get_byte(BitVector *v, uint32_t i) {
    return v->vector[i];
}

// Debugger function to print bit vector
void bv_print(BitVector *v) {
    for (uint32_t i = 0; i < v->length; i++) {
        printf("bv[%d]=%d \n", i, v->vector[i]);
    }
    printf("\n");
    return;
}
