#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "dataclass.h"
#include "create.h"
#include "print.h"
#include "math.h"


#define ASSERT(cond, msg)                     \
    do {                                     \
        if (!(cond)) {                       \
            fprintf(stderr, "[FAIL] %s\n", msg); \
            return 1;                        \
        }                                    \
    } while (0)

static int assert_elem(const Matrix *m, int i, int j, float esperado) {
    float v = -1.0f;
    if (matrix_getelem(m, i, j, &v) != 0) return 1;
    return 0;
}

int main(void) {
    Matrix *A = NULL;
    Matrix *B = NULL;
    Matrix *C = NULL;

    /* ---------- TESTE: criação ---------- */
    A = init_matrix(2, 2);
    B = init_matrix(2, 2);
    ASSERT(A && B, "Falha ao criar matrizes A ou B");

    /* ---------- TESTE: inserção ---------- */
    ASSERT(matrix_setelem(A, 1, 1, 1.0f) == 0, "Falha set A(1,1)");
    ASSERT(matrix_setelem(A, 1, 2, 2.0f) == 0, "Falha set A(1,2)");
    ASSERT(matrix_setelem(A, 2, 1, 3.0f) == 0, "Falha set A(2,1)");

    ASSERT(matrix_setelem(B, 1, 1, 4.0f) == 0, "Falha set B(1,1)");
    ASSERT(matrix_setelem(B, 2, 2, 5.0f) == 0, "Falha set B(2,2)");

    /* ---------- TESTE: get ---------- */
    ASSERT(assert_elem(A, 1, 1, 1.0f) == 0, "Erro get A(1,1)");
    ASSERT(assert_elem(A, 1, 2, 2.0f) == 0, "Erro get A(1,2)");
    ASSERT(assert_elem(A, 2, 1, 3.0f) == 0, "Erro get A(2,1)");
    ASSERT(assert_elem(A, 2, 2, 0.0f) == 0, "Erro get A(2,2)");

    /* ---------- TESTE: soma ---------- */
    ASSERT(matrix_add(A, B, &C) == 0, "Falha em matrix_add");

    ASSERT(assert_elem(C, 1, 1, 5.0f) == 0, "Erro C(1,1)");
    ASSERT(assert_elem(C, 1, 2, 2.0f) == 0, "Erro C(1,2)");
    ASSERT(assert_elem(C, 2, 1, 3.0f) == 0, "Erro C(2,1)");
    ASSERT(assert_elem(C, 2, 2, 5.0f) == 0, "Erro C(2,2)");

    matrix_destroy(C);
    C = NULL;

    /* ---------- TESTE: transposta ---------- */
    ASSERT(matrix_transpose(A, &C) == 0, "Falha em matrix_transpose");

    ASSERT(assert_elem(C, 1, 1, 1.0f) == 0, "Erro AT(1,1)");
    ASSERT(assert_elem(C, 2, 1, 2.0f) == 0, "Erro AT(2,1)");
    ASSERT(assert_elem(C, 1, 2, 3.0f) == 0, "Erro AT(1,2)");

    matrix_destroy(C);
    C = NULL;

    /* ---------- TESTE: multiplicação ---------- */
    ASSERT(matrix_multiply(A, B, &C) == 0, "Falha em matrix_multiply");

    ASSERT(assert_elem(C, 1, 1, 4.0f) == 0, "Erro A*B (1,1)");
    ASSERT(assert_elem(C, 1, 2, 10.0f) == 0, "Erro A*B (1,2)");
    ASSERT(assert_elem(C, 2, 1, 12.0f) == 0, "Erro A*B (2,1)");
    ASSERT(assert_elem(C, 2, 2, 0.0f) == 0, "Erro A*B (2,2)");

    matrix_destroy(C);
    matrix_destroy(A);
    matrix_destroy(B);

    printf("[OK] Todos os testes passaram com sucesso.\n");
    return 0;
}
