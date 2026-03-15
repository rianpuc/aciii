#include <stdlib.h>
#include <gem5/m5ops.h>
#include "graph_generator/graph_generator.h"

void printf();

Lista *initializeLista() {
    Lista *lista = malloc(sizeof(Lista));
    lista->head = NULL;
    lista->tail = NULL;
    return lista;
}

Node *createNode(int n) {
    Node *node = malloc(sizeof(Node));
    node->id = n;
    node->next = NULL;
    return node;
}

void enqueue(Lista *lista, int n) {
    Node *node = createNode(n);
    if (lista->head == NULL) {
        lista->head = node;
        lista->tail = node;
    } else  {
        lista->tail->next = node;
        lista->tail = node;
    }
}

Node *dequeue(Lista *lista) {
    if (lista->head == NULL) { return NULL; }
    Node *node = lista->head;
    lista->head = lista->head->next;
    if (lista->head == NULL) {
        lista->tail = NULL;
    }
    return node;
}

void bfs(Graph *graph, unsigned int start_vertex, unsigned int num_vertices) {
    int *visited = calloc(num_vertices, sizeof(int));
    Lista *queue = initializeLista();

    visited[start_vertex] = 1;
    enqueue(queue, start_vertex);

    while (queue->head != NULL) {
        Node *current_node = dequeue(queue);
        int u = current_node->id;
        free(current_node);
        Node *vizinho = graph->v[u].head;
        while (vizinho != NULL) {
            int v = vizinho->id;
            if (!visited[v]) {
                visited[v] = 1;
                enqueue(queue, v);
            }
            vizinho = vizinho->next;
        }
    }

    free(visited);
    free(queue);
}

void parse_args(unsigned int argc, char *argv[], unsigned int *vertices, unsigned int *density, int *seed) {
    if (argc != 4) {
        printf("Proper usage: ./program VERTICES DENSITY SEED");
        exit(1);
    }
    if (atoi(argv[1]) <= 0) {
        printf("VERTICES must be positive or greater than 0.\n");
        exit(1);
    }
    if (atoi(argv[2]) < 0 || atoi(argv[2]) > 2) {
        printf("DENSITY must be a number between 0 and 2.\n");
        exit(1);
    }
    *vertices = atoi(argv[1]);
    *density = atoi(argv[2]);
    *seed = atoi(argv[3]);
}

int main(unsigned int argc, char **argv) {
    unsigned int vertices, density;
    int seed;
    parse_args(argc, argv, &vertices, &density, &seed);
    srand(seed);
    Graph graph = createGraph(vertices, density);
    m5_reset_stats(0, 0);
    bfs(&graph, 0, vertices);
    m5_dump_stats(0, 0);
    return 0;
}