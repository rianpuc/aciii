#include "graph_generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

Graph initializeGraph(unsigned int vertices) // funcao para inicializar o grafo
{
    Graph grafo;
    grafo.vc = vertices;
    grafo.v = malloc(sizeof(Lista) * vertices);
    for (int i = 0; i < vertices; i++) // inicializando tudo como null
    {
        grafo.v[i].head = grafo.v[i].tail = NULL;
    }
    return grafo;
}

void addEdgeToList(Lista *list, int id)
{
    Node *novo = malloc(sizeof(Node));
    novo->id = id;
    novo->next = NULL;
    if (list->head == NULL)
    {
        list->head = list->tail = novo;
    }
    else
    {
        list->tail->next = novo;
        list->tail = novo;
    }
}

void addEdge(Graph *graph, int u, int v)
{
    addEdgeToList(&graph->v[u], v);
    addEdgeToList(&graph->v[v], u);
}

void populateGraph(Graph *graph, unsigned int vertices, Densidade d)
{
    double prob = 0;
    switch (d)
    {
        case ESPARSO: prob = 0.05; break;
        case MEDIO:   prob = 0.15; break;
        case DENSO:   prob = 0.30; break;
        default:      prob = 0.05; break;
    }

    int prob_limit = (int)(prob * RAND_MAX);
    //garantir conexo
    for (int i = 0; i < vertices - 1; i++)
    {
        addEdge(graph, i, i+1);
    }
    //aleatoridade (densidade)
    for (int i = 0; i < vertices; i++) {
        for (int j = i + 2; j < vertices; j++) {
            if (rand() <= prob_limit) {
                addEdge(graph, i, j);
            }
        }
    }

}

void printGraph(Graph *graph) {
    for (int i = 0; i < graph->vc; i++) {
        printf("%d: ", i);
        Node *ptr = graph->v[i].head;
        while (ptr != NULL) {
            printf("%d -> ", ptr->id);
            ptr = ptr->next;
        }
        printf("\n");
    }
}

Graph createGraph(unsigned int vertices, Densidade d)
{
    Graph graph = initializeGraph(vertices);
    populateGraph(&graph, vertices, d);
    return graph;
}
