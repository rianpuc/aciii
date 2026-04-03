#include <stdlib.h>
#include <gem5/m5ops.h>

void printf();
int** alocarMatriz(int n, int m);
void preencherMatriz(int **matriz, int n, int m, int max);
void freeMatriz(int **matriz, int n, int m);
void multiplicarMatrizes(int **matrizA, int **matrizB, int **matrizResult, int n);

int** alocarMatriz(int n, int m) {
    int **matriz = (int**) malloc(n * sizeof(int*));
    for(int i = 0; i < n; i++) {
        matriz[i] = (int*) malloc(m * sizeof(int));
    }
    return matriz;
}

// n = linhas
// m = colunas
// valor máximo possível de um elemento
int** gerarMatriz(int n, int m, int max) {
    int **matriz = alocarMatriz(n, m);
    preencherMatriz(matriz, n, m, max);
    
    return matriz;
}

void preencherMatriz(int **matriz, int n, int m, int max) {
    if (matriz == NULL) {
        printf("Erro de memoria\n");
        exit(1);
    }

    for(int i = 0; i < n; i++) {
        for(int j = 0; j < m; j++) {
            matriz[i][j] = rand() % max;
        }
    }
}

void freeMatriz(int **matriz, int n, int m) {
    for(int i = 0; i < n; i++) {
        free(matriz[i]);
    }
    free(matriz);
}

void multiplicarMatrizes(int **matrizA, int **matrizB, int **matrizResult, int n) {
    for(int i = 0; i < n; i++) {
        for(int k = 0; k < n; k++) { // ordem do loop alterada
            int soma = 0;
            for(int j = 0; j < n; j++) { // o j vira interno
                soma += matrizA[i][k] * matrizB[k][j]; // o acesso a matrizA eh constante, o que varia eh o indice j (linha), logo Row-Major
            }
            matrizResult[i][k] = soma;
        }
    }
}

void parse_args(unsigned int argc, char **argv, unsigned int *n, int *seed) {
    if (argc != 3) {
        printf("Proper usage: ./program MATRIX_SIZE SEED");
        exit(1);
    }
    if (atoi(argv[1]) <= 0) {
        printf("MATRIX_SIZE must be positive or greater than 0.\n");
        exit(1);
    }
    *n = atoi(argv[1]);
    *seed = atoi(argv[2]);
}

int main(int argc, char **argv) {
    unsigned int n;
    int seed;
    parse_args(argc, argv, &n, &seed);
    srand(seed);

    int **matrizA = gerarMatriz(n, n, rand());
    int **matrizB = gerarMatriz(n, n, rand());
    int **matrizResult = alocarMatriz(n, n);

    m5_reset_stats(0, 0);
    multiplicarMatrizes(matrizA, matrizB, matrizResult, n);
    m5_dump_stats(0, 0);

    // liberar memória
    freeMatriz(matrizA, n, n);
    freeMatriz(matrizB, n, n);
    freeMatriz(matrizResult, n, n);
    return 0;
}