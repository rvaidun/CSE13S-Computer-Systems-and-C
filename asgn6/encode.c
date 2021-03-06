#include "defines.h"
#include "header.h"
#include "huffman.h"
#include "io.h"

#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h> // For getopt
#define OPTIONS "hvi:o:"

// Prints the help message
void print_help(void) {
    printf("SYNOPSIS\n"
           "   A Huffman encoder\n"
           "   Compress a file using the Huffman coding algorithm\n"
           "USAGE\n"
           "   ./encode [-h] [-v] [-i infile] [-o outfile]\n\n"
           "OPTIONS\n"
           "   -h             Display program help and usage.\n"
           "   -i infile      Input data to compress (default: stdin)\n"
           "   -o outfile     Output of compressed data (default: stdout)\n"
           "   -v             Print compression statistics.\n");

    return;
}

// Debug function to print the histogram
void print_histogram(uint64_t *hist) {
    for (int i = 0; i < ALPHABET; i++) {
        if (hist[i] != 0) {
            printf("character: %c, amount: %" PRIu64 "\n", i, hist[i]);
        }
    }
}

// Postorder traversal function
void postorder_traversal(Node *root, uint8_t *arr, uint32_t *i) {
    if (root) {
        postorder_traversal(root->left, arr, i);
        postorder_traversal(root->right, arr, i);
        if (root->left == NULL && root->right == NULL) {
            arr[*i] = 'L';
            *i = *i + 1;
            arr[*i] = root->symbol;
            *i = *i + 1;
        } else {
            arr[*i] = 'I';
            *i = *i + 1;
        }
    }
}

int main(int argc, char **argv) {
    int br;
    Header h;
    Node *root;
    struct stat instatbuf;
    uint8_t dump[MAX_TREE_SIZE];
    uint8_t buf[BLOCK];
    uint16_t unique_symbols = 0;
    uint32_t dump_index = 0;
    int infile = 0;
    int outfile = 1;
    bool verbose = false;
    Code table[ALPHABET] = { 0 };
    int tempfiled = 0;

    // Initialize all values of histogram to 0 and increment index 0 and 255
    uint64_t hist[ALPHABET];
    for (int i = 0; i < ALPHABET; i++) {
        hist[i] = 0;
    }
    hist[0]++;
    hist[255]++;

    // Parse command line options
    int opt = 0;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'h': print_help(); return -1;
        case 'i':
            infile = open(optarg, O_RDONLY);
            if (infile == -1) {
                printf("Error opening file\n");
                return -1;
            }
            break;
        case 'o':
            outfile = open(optarg, O_WRONLY | O_CREAT);
            if (outfile == -1) {
                printf("Error opening file\n");
                return -1;
            }
            break;
        case 'v': verbose = true; break;
        default: print_help(); return -1;
        }
    }

    // If the infile is stdio write to a temporary file and then so we can seek
    if (lseek(infile, 0, SEEK_SET) == -1) {
        tempfiled = open("/tmp/encode.temporary", O_CREAT | O_RDWR | O_TRUNC, 0600);

        while ((br = read_bytes(infile, buf, BLOCK)) > 0) {
            write_bytes(tempfiled, buf, br);
        }

        infile = tempfiled;
    }
    lseek(infile, 0, SEEK_SET);

    // Get the stats for the infile and change mode
    fstat(infile, &instatbuf);
    fchmod(outfile, instatbuf.st_mode);

    // Create the histogram
    while ((br = read_bytes(infile, buf, BLOCK)) > 0) {
        for (int i = 0; i < br; i++) {
            hist[buf[i]]++;
        }
    }

    // Count the unique symbols
    for (int i = 0; i < ALPHABET; i++) {
        if (hist[i] > 0) {
            unique_symbols++;
        }
    }

    // Build root and codes
    root = build_tree(hist);
    build_codes(root, table);

    // Build header and write it
    h.magic = MAGIC;
    h.permissions = instatbuf.st_mode;
    h.tree_size = (3 * unique_symbols) - 1;
    h.file_size = instatbuf.st_size;
    write_bytes(outfile, (uint8_t *) &h, sizeof(Header));

    // Post order traversal through the tree to make a tree dump buffer
    postorder_traversal(root, dump, &dump_index);
    write_bytes(outfile, dump, dump_index);

    // Seek back to 0, loop through each byte and write the corresponding code
    lseek(infile, 0, SEEK_SET);
    while ((br = read_bytes(infile, buf, BLOCK)) > 0) {
        for (int i = 0; i < br; i++) {
            write_code(outfile, &table[buf[i]]);
        }
    }
    flush_codes(outfile);

    if (verbose) {
        fprintf(stderr, "Uncompressed file size: %" PRIu64 " bytes\n", instatbuf.st_size);
        fprintf(stderr, "Compressed file size: %" PRIu64 " bytes\n", bytes_written);
        fprintf(stderr, "Space saving: %.2f%%\n",
            100 * (1 - ((double) bytes_written / instatbuf.st_size)));
    }

    if (tempfiled) {
        unlink("/tmp/encode.temporary");
    }
    close(infile);
    close(outfile);
    delete_tree(&root);
}
