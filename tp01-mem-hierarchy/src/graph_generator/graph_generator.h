#ifndef __GRAPH_GENERATOR_H__
#define __GRAPH_GENERATOR_H__
typedef enum
{
    ESPARSO,
    MEDIO,
    DENSO
} Densidade;

typedef struct Node
{
    struct Node *next;
    int id;
} Node;

typedef struct
{
    Node *head;
    Node *tail;
} Lista;

typedef struct
{
    Lista *v; // lista de adjacencia normal
    unsigned int vc; // quantidade de vertices no grafo
} Graph;

Graph createGraph(unsigned int vertices, Densidade d);
void printGraph(Graph *graph);
#endif