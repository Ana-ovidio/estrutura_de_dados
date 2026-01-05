#include <stdio.h>
#include "print.h"
#include <math.h>

void matrix_print(const Matrix *m) {
    if (!m) return;

    printf("\nMatriz esparsa (%d x %d)\n", m->linhas, m->colunas);

    for (int i = 0; i < m->linhas; i++) {
        printf("Linha %d:", i + 1);

        POINT atual = m->mat[i];
        if (!atual) printf(" [vazia]");

        while (atual) {
            printf(" (%d, %.2f)", atual->coluna, atual->valor);
            atual = atual->prox;
        }
        printf("\n");
    }
}

void matrix_print_labeled(const char *label, const Matrix *m) {
    if (label) printf("\n=== %s ===\n", label);
    matrix_print(m);
}
