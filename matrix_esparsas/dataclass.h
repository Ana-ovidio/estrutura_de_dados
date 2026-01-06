#ifndef DATACLASS_H
#define DATACLASS_H

typedef struct No {
    int coluna;
    float valor;
    struct No *prox;
} No;

typedef No* POINT;

typedef struct Matrix {
    int linhas;
    int colunas;
    POINT *mat;
} Matrix;

#endif
