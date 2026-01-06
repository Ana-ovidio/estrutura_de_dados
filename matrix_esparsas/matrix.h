#ifndef MATRIX_H
#define MATRIX_H

/* ===== Criação / Destruição ===== */

Matrix* init_matrix(int linhas, int colunas);
int matrix_destroy(Matrix *m);
int matrix_destroy_labeled(const char *label, Matrix *m);

int matrix_create(Matrix **m);
int matrix_create_labeled(const char *label, Matrix **m);

/* ===== Entrada / Saída ===== */

void input_tam_matrix(int *linhas, int *colunas);
int insert_matrix_stdin(const char *label, Matrix *m);

void matrix_print(const Matrix *m);
void matrix_print_labeled(const char *label, const Matrix *m);

/* ===== Acesso a elementos ===== */

int matrix_setelem(Matrix *m, int i, int j, float valor);
int matrix_addelem(Matrix *m, int i, int j, float delta);
int matrix_getelem(const Matrix *m, int x, int y, float *elem);

/* ===== Operações ===== */

int matrix_add(const Matrix *m, const Matrix *n, Matrix **r);
int matrix_transpose(const Matrix *m, Matrix **r);
int matrix_multiply(const Matrix *m, const Matrix *n, Matrix **r);

#endif
