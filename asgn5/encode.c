#include "bm.h"
#include "hamming.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h> // For getopt
#define OPTIONS "hi:o:"

// Prints the help message
void print_help(void) {
    printf("SYNOPSIS\n"
           "   A Hamming(8, 4) systematic code generator.\n"
           "USAGE\n"
           "   ./encode [-h] [-i infile] [-o outfile]\n\n"
           "OPTIONS\n"
           "   -h              Display program help and usage.\n"
           "   -i infile      Input data to encode (default: stdin)\n"
           "   -o outfile     Output of encoded data (default: stdout)\n");
    return;
}
// Code from assignment PDF
uint8_t lower_nibble(uint8_t val) {
    return val & 0xF;
}

// Code from assignment PDF
uint8_t upper_nibble(uint8_t val) {
    return val >> 4;
}

int main(int argc, char **argv) {
    // Set defaults
    FILE *in_fp = stdin;
    FILE *out_fp = stdout;
    struct stat statbuf;
    uint8_t encode_table[16];
    int c; // C is where we will store the Code from fgetc

    // Parse command line arguments
    int opt = 0;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i':
            in_fp = fopen(optarg, "r");
            if (in_fp == NULL) {
                fprintf(stderr, "Error: failed to open infile.\n");
                return -1;
            }
            break;

        case 'o':
            out_fp = fopen(optarg, "wb");
            if (out_fp == NULL) {
                fprintf(stderr, "Error opening file to write\n");
                return -1;
            }
            break;
        default: print_help(); return -1;
        }
    }

    // Code to change file permissions from assignment PDF
    fstat(fileno(in_fp), &statbuf);
    fchmod(fileno(out_fp), statbuf.st_mode);

    // Create the Generator matrix based on the matrix in the assignment PDF
    BitMatrix *G = bm_create(4, 8);
    bm_set_bit(G, 0, 0);
    bm_set_bit(G, 0, 5);
    bm_set_bit(G, 0, 6);
    bm_set_bit(G, 0, 7);
    bm_set_bit(G, 1, 1);
    bm_set_bit(G, 1, 4);
    bm_set_bit(G, 1, 6);
    bm_set_bit(G, 1, 7);
    bm_set_bit(G, 2, 2);
    bm_set_bit(G, 2, 4);
    bm_set_bit(G, 2, 5);
    bm_set_bit(G, 2, 7);
    bm_set_bit(G, 3, 3);
    bm_set_bit(G, 3, 4);
    bm_set_bit(G, 3, 5);
    bm_set_bit(G, 3, 6);

    // Populate the lookup table for the generator matrix
    for (int i = 0; i < 16; i++) {
        encode_table[i] = ham_encode(G, i);
    }

    while ((c = fgetc(in_fp)) != EOF) {
        fputc(encode_table[lower_nibble(c)], out_fp); // Put the lower nibble
        fputc(encode_table[upper_nibble(c)], out_fp); // Put the upper nibble
    }
    // Free up any data that was allocated
    fclose(in_fp);
    fclose(out_fp);
    bm_delete(&G);
    return 0;
}
