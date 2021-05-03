#include "bv.h"

#include <stdlib.h>
struct BitVector {
    uint32_t length;
    uint8_t *vector;
};

BitVector *bv_create(uint32_t length) {
    BitVector *bv = (BitVector *) malloc(sizeof(BitVector));
    bv->length = length;
    bv->vector = (uint8_t *) calloc(length, sizeof(uint8_t));
}

void bv_delete(BitVector **v) {
    if (*v && (*v)->vector) {
        free((*v)->vector);
        free(*v);
        *v = NULL;
    }
}

uint32_t bv_length(BitVector *v) {
    return v->length;
}

void bv_set_bit(BitVector *v, uint32_t i) {
    uint32_t bytepos = i / 8;
    uint32_t bitpos = i % 8;
    v->vector[bytepos] |= (1 << bitpos);
    return;
}

void bv_clr_bit(BitVector *v, uint32_t i) {
    uint32_t bytepos = i / 8;
    uint32_t bitpos = i % 8;
    v->vector[bytepos] &= ~(1 << bitpos);
    return;
}

uint8_t bv_get_bit(BitVector *v, uint8_t i) {
    uint32_t bytepos = i / 8;
    uint32_t bitpos = i % 8;
    return (v->vector[bytepos] >> bitpos) & 1;
}

void bv_xor_bit(BitVector *v, uint32_t i, uint8_t bit) {
    uint32_t bytepos = i / 8;
    uint32_t bitpos = i % 8;
    uint8_t b = bv_get_bit(v, i);
    v->vector[bytepos] = (v->vector[bytepos] & (~(1 << bitpos))) | ((b ^ bit) << bitpos);
}

void bv_print(BitVector *v) {
    for (int i = 0; i < v->length; i++) {
        printf("%d ", v->vector[i]);
    }
    printf("\n");
}