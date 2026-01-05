#ifndef CREATE_H
#define CREATE_H

#include "dataclass.h"

Matrix* init_matrix(int linhas, int colunas);
int matrix_setelem(Matrix *m, int i, int j, float valor);
int matrix_create(Matrix **m);
int matrix_destroy(Matrix *m);
int matrix_getelem(const Matrix *m, int x, int y, float *elem);

int matrix_create_labeled(const char *label, Matrix **m);
int matrix_destroy_labeled(const char *label, Matrix *m);

#endif
