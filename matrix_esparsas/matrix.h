#ifndef MATRIX_H
#define MATRIX_H

typedef struct matrix Matrix;

/* ===== Criação / Destruição ===== */
int matrix_create(Matrix **m);
int matrix_destroy(Matrix *m);

/* ===== Saída ===== */
int matrix_print(const Matrix *m);

/* ===== Acesso a elementos ===== */
int matrix_getelem(const Matrix *m, int x, int y, float *elem);
int matrix_setelem(Matrix *m, int x, int y, float elem);

/* ===== Operações ===== */
int matrix_add(const Matrix *m, const Matrix *n, Matrix **r);
int matrix_multiply(const Matrix *m, const Matrix *n, Matrix **r);
int matrix_transpose(const Matrix *m, Matrix **r);

#endif
