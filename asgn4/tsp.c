#include "graph.h"
#include "path.h"
#include "vertices.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define OPTIONS "vui:o:"

// Depth first search algorithm to find the shortest hamiltonian path
// G - Graph
// v - current vertex
// curr - pointer to the current pah
// shortest - pointer to the shortest path
//outfile - file pointer to write to
//verbose - boolean to say wheather verbose enabled
// Rec - pointer to keep track of number of calls to dfs
void dfs(Graph *G, uint32_t v, Path *curr, Path *shortest, char *cities[], FILE *outfile,
    bool verbose, uint32_t *rec) {
    *rec = *rec + 1; // Increment calls by 1
    uint32_t popped = 0; // Arbitrary variable  to hold the popped numbers when popping from path
    // Mark visited and push to path
    graph_mark_visited(G, v);
    path_push_vertex(curr, v, G);
    // not_too_long finds if the current path is already too long
    // If path too long no need to go into for loop
    bool not_too_long = (path_length(shortest) == 0) || path_length(curr) < path_length(shortest);

    for (uint32_t w = 0; w < graph_vertices(G) && not_too_long; w++) {
        if (graph_has_edge(G, v, w)) {

            if (!graph_visited(G, w)) {
                dfs(G, w, curr, shortest, cities, outfile, verbose, rec);

                // if vertex has been visited but vertex == the start vertex then check if
                // current path length is number of vertices in the grpah (found hamiltonian path)
                // If all conditions are met we can push the start vertex
            } else if ((w == START_VERTEX) && (path_vertices(curr) == graph_vertices(G))) {

                path_push_vertex(curr, w, G);
                // Check current path shorter than the shortest current path (path not too long)
                if (path_length(shortest) == 0 || path_length(curr) < path_length(shortest)) {

                    path_copy(shortest, curr);
                    // Check if verbose to print
                    if (verbose) {
                        path_print(curr, outfile, cities);
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

// Help message
void print_help(void) {
    printf("SYNOPSIS\n"
           "   Traveling Salesman Problem using DFS.\n"
           "USAGE\n"
           "./tsp [-u] [-v] [-h] [-i infile] [-o outfile]\n\n"
           "OPTIONS\n"
           "   -h              Display program help and usage.\n"
           "   -u             Use undirected graph.\n"
           "   -v             Enable verbose printing.\n"
           "   -i infile      Input containing graph (default: stdin)\n"
           "   -o outfile     Output of computed path (default: stdout)\n");
    return;
}
// Main function for tsp
int main(int argc, char **argv) {
    // Set defaults and initialize all variables
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

    // Handle command like arguments
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {

        case 'v': verbose = true; break;
        case 'u': undirected = true; break;
        case 'i':
            in_fp = fopen(optarg, "r");
            if (in_fp == NULL) {
                fprintf(stderr, "Error: failed to open infile.\n");
                return -1;
            }
            break;

        case 'o':
            out_fp = fopen(optarg, "w");
            if (out_fp == NULL) {
                fprintf(stderr, "Error opening file to write\n");
                return -1;
            }
            break;
        default: print_help(); return -1;
        }
    }

    // Check for valid number of cities
    if (fscanf(in_fp, "%d\n", &num_cities) != 1 || num_cities > VERTICES) {
        fprintf(stderr, "Error: malformed number of vertices.\n");
        return -1;
    }

    if (num_cities <= 1) {
        printf("There's nowhere to go.\n");
        return 0;
    }

    // Allocate space for graph and make number of cities
    graph = graph_create(num_cities, undirected);
    cities = malloc(num_cities * sizeof(char *));

    // Store cities to a cities array
    for (uint32_t i = 0; i < num_cities; i++) {
        if (fgets(buffer, 1024, in_fp) != NULL) {
            strtok(buffer, "\n");
            cities[i] = strdup(buffer);
        }
    }

    // Read all the edges with the weights and store in the graph
    while ((temp = fscanf(in_fp, "%d %d %d", &i, &j, &k)) != EOF) {
        if (temp == 3) {
            graph_add_edge(graph, i, j, k);

        } else {
            fprintf(stderr, "Error: malformed edge.\n");
            return -1;
        }
    }

    // Create two paths for current and short path
    cur_path = path_create();
    short_path = path_create();

    dfs(graph, START_VERTEX, cur_path, short_path, cities, out_fp, verbose, &recursive_calls);

    // If Hamiltonian path found print the path else print none were found
    if (path_length(short_path) > 0) {
        path_print(short_path, out_fp, cities);
    } else {
        printf("No Hamiltonian path found.\n");
    }

    fprintf(out_fp, "Total recursive calls: %d\n", recursive_calls);

    // Free memory and handle memory leaks
    path_delete(&cur_path);
    path_delete(&short_path);
    graph_delete(&graph);
    for (uint32_t i = 0; i < num_cities; i++) {
        free(cities[i]);
    }
    free(cities);
    fclose(in_fp);
    fclose(out_fp);
    return 0;
}
