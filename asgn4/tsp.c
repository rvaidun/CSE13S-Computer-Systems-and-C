#include "graph.h"
#include "path.h"
#include "vertices.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define OPTIONS "vui:o:"

void dfs(Graph *G, uint32_t v, Path *curr, Path *shortest, char *cities[], FILE *outfile,
    bool verbose, uint32_t *rec) {
    *rec = *rec + 1;
    uint32_t popped = 0;
    graph_mark_visited(G, v);
    path_push_vertex(curr, v, G);
    bool nottoolong = (path_length(shortest) == 0) || path_length(curr) < path_length(shortest);
    for (uint32_t w = 0; w < graph_vertices(G) && nottoolong; w++) {
        if (graph_has_edge(G, v, w)) {
            if (!graph_visited(G, w)) {
                dfs(G, w, curr, shortest, cities, outfile, verbose, rec);
            } else if (w == START_VERTEX) {
                path_push_vertex(curr, w, G);
                if (path_vertices(curr) == graph_vertices(G) + 1) {
                    if (path_length(shortest) == 0 || path_length(curr) < path_length(shortest)) {
                        path_copy(shortest, curr);
                        if (verbose) {
                            path_print(curr, outfile, cities);
                        }
                    }
                }
                path_pop_vertex(curr, &popped, G);
            }
        }
    }
    path_pop_vertex(curr, &popped, G);
    graph_mark_unvisited(G, v);
    return;
}
int main(int argc, char **argv) {
    bool verbose = false;
    bool undirected = false;
    FILE *in_fp = stdin;
    FILE *out_fp = stdout;
    uint32_t recursive_calls = 0;
    uint32_t num_cities;
    char **cities;
    char buffer[1024];
    uint32_t i, j, k;
    int temp;
    Graph *graph;
    Path *cur_path;
    Path *short_path;
    int opt = 0;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'v': verbose = true; break;
        case 'u': undirected = true; break;
        case 'i':
            in_fp = fopen(optarg, "r");
            if (in_fp == NULL) {
                printf("Error: failed to open infile.\n");
                return -1;
            }
            break;
        case 'o':
            out_fp = fopen(optarg, "w");
            if (out_fp == NULL) {
                printf("Error opening file to write\n");
                return -1;
            }
            break;
        }
    }
    if (fscanf(in_fp, "%d\n", &num_cities) != 1) {
        printf("Error: malformed number of vertices.\n");
        return -1;
    }
    if (num_cities <= 1) {
        printf("There's nowhere to go.\n");
        return 0;
    }
    graph = graph_create(num_cities, undirected);
    cities = malloc(num_cities * sizeof(char *));

    for (uint32_t i = 0; i < num_cities; i++) {
        if (fgets(buffer, 1024, in_fp) != NULL) {
            strtok(buffer, "\n");
            cities[i] = strdup(buffer);
        }
    }

    while ((temp = fscanf(in_fp, "%d %d %d", &i, &j, &k)) != EOF) {
        if (temp == 3) {
            graph_add_edge(graph, i, j, k);

        } else {
            printf("Error: malformed edge.\n");
            return -1;
        }
    }
    cur_path = path_create();
    short_path = path_create();
    dfs(graph, START_VERTEX, cur_path, short_path, cities, out_fp, verbose, &recursive_calls);

    path_print(short_path, out_fp, cities);
    fprintf(out_fp, "Total recursive calls: %d\n", recursive_calls);
}