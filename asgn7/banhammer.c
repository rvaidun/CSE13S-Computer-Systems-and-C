// 'look .nr x0 n(.l
#include "bf.h"
#include "ht.h"
#include "ll.h"
#include "llnode.h"
#include "messages.h"
#include "node.h"
#include "parser.h"

#include <ctype.h>
#include <inttypes.h>
#include <math.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For getopt

#define OPTIONS "hmst:f:"
#define WORD    "[a-zA-Z0-9_]+(('|-)[a-zA-Z0-9_]+)*"
uint64_t seeks = 0;
uint64_t links = 0;

void print_help(void) {
    printf("SYNOPSIS\n"
           "   A word filtering program for the GPRSC.\n"
           "   Filters out and reports bad words parsed from stdin.\n"
           "USAGE\n"
           "   ./banhammer [-hsm] [-t size] [-f size]\n\n"
           "OPTIONS\n"
           "   -h             Display program help and usage.\n"
           "   -s           Print program statistics.\n"
           "   -m           Enable move-to-front rule.\n"
           "   -t size      Specify hash table size (default: 10000).\n"
           "   -f size      Specify Bloom filter size (default: 2^20).\n");
    return;
}
int main(int argc, char **argv) {
    regex_t re;
    if (regcomp(&re, WORD, REG_EXTENDED)) {
        fprintf(stderr, "regex broken\n");
        return 1;
    }
    FILE *bspkf = fopen("badspeak.txt", "r");
    if (!bspkf) {
        fprintf(
            stderr, "Failed to open badspeak.txt. File needs to be in current working directory\n");
    }

    FILE *nspkf = fopen("newspeak.txt", "r");
    if (!nspkf) {
        fprintf(
            stderr, "Failed to open newspeak.txt. File needs to be in current working directory");
    }

    uint32_t badspeaklength;
    uint32_t translationslength;
    BloomFilter *bf;
    HashTable *ht;
    char buffer[1024];
    char buffer2[1024];
    char *word;
    char *first_invalid;
    Node *n;
    int temp;

    bool stats = false;
    bool mtf = false;
    uint32_t size_bf = (uint32_t) pow(2, 20);
    uint32_t size_ht = 10000;
    int opt = 0;
    printf("Before out of switch case\n");
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'h': print_help(); return -1;
        case 's': stats = true; break;
        case 'm': mtf = true; break;

        case 't':
            size_ht = strtoul(optarg, &first_invalid, 10);
            if (*first_invalid != '\0') {
                return -1;
            }
            break;

        case 'f':
            size_bf = strtoul(optarg, &first_invalid, 10);
            if (*first_invalid != '\0') {
                return -1;
            }
            break;
        default: print_help(); break;
        }
    }
    printf("Came out of switch case\n");
    LinkedList *badspeakwords = ll_create(mtf);
    LinkedList *translations = ll_create(mtf);
    // Initialize bloom filter and hash table
    bf = bf_create(size_bf);
    ht = ht_create(size_ht, mtf);
    printf("Created bf and ht\n");
    while ((temp = fscanf(bspkf, "%s", buffer)) != EOF) {
        bf_insert(bf, buffer);
        ht_insert(ht, buffer, NULL);
    }
    printf("Inserted everything bf and ht\n");
    while ((temp = fscanf(nspkf, "%s %s", buffer, buffer2)) != EOF) {
        bf_insert(bf, buffer);
        ht_insert(ht, buffer, buffer2);
    }
    printf("Inserted everything two\n");

    while ((word = next_word(stdin, &re)) != NULL) {
        printf("In while\n");
        for (int i = 0; word[i]; i++) {
            word[i] = tolower(word[i]);
        }
        if (bf_probe(bf, word)) {
            printf("Probe is true\n");
            n = ht_lookup(ht, word);
            printf("Returned ht lookup\n");
            if (n != NULL) {
                if (n->newspeak && n->oldspeak) {
                    ll_insert(translations, n->oldspeak, n->newspeak);
                } else if (n->oldspeak) {
                    ll_insert(badspeakwords, n->oldspeak, NULL);
                }
            }
        }
    }
    printf("Finished while\n");
    if (!stats) {

        badspeaklength = ll_length(badspeakwords);
        translationslength = ll_length(translations);
        if (badspeaklength > 0 && translationslength > 0) {
            fprintf(stdout, "%s", mixspeak_message);
            ll_print(badspeakwords);
            ll_print(translations);
        } else if (badspeaklength > 0) {
            fprintf(stdout, "%s", badspeak_message);
            ll_print(badspeakwords);
        } else if (translationslength > 0) {
            fprintf(stdout, "%s", goodspeak_message);
            ll_print(translations);
        }

    } else {
        fprintf(stdout, "Seeks: %" PRIu64 "\n", seeks);
        fprintf(stdout, "Average seek length: %f\n", ((double) links / seeks));
        fprintf(stdout, "Hash table load: %f%%\n", 100 * ((double) ht_count(ht) / ht_size(ht)));
        fprintf(stdout, "Bloom filter load: %f%%\n", 100 * ((double) bf_count(bf) / bf_size(bf)));
    }

    fclose(bspkf);
    fclose(nspkf);
    ll_delete(&badspeakwords);
    ll_delete(&translations);
    bf_delete(&bf);
    ht_delete(&ht);
    clear_words();
    regfree(&re);
    return 0;
}
