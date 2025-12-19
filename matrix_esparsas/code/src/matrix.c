#include <stdio.h>
#include <stdlib.h>

#include "dataclass.h"
#include "create.h"
#include "print.h"
#include "math.h"

int main(void) {
    Matrix *A = NULL;
    Matrix *B = NULL;
    Matrix *C = NULL;

    if (matrix_create_labeled("A", &A) == 0) {
        matrix_print_labeled("A (entrada)", A);
    } else {
        fprintf(stderr, "Erro ao criar A.\n");
        return 1;
    }

    if (matrix_create_labeled("B", &B) == 0) {
        matrix_print_labeled("B (entrada)", B);
    } else {
        fprintf(stderr, "Erro ao criar B.\n");
        matrix_destroy_labeled("A", A);
        return 1;
    }

    printf("\n[OP] C = A + B\n");
    if (matrix_add(A, B, &C) == 0) {
        matrix_print_labeled("C = A + B", C);
        matrix_destroy_labeled("C", C);
        C = NULL;
    } else {
        fprintf(stderr, "Erro na soma C = A + B.\n");
    }

    printf("\n[OP] C = A * B\n");
    if (matrix_multiply(A, B, &C) == 0) {
        matrix_print_labeled("C = A * B", C);
        matrix_destroy_labeled("C", C);
        C = NULL;
    } else {
        fprintf(stderr, "Erro na multiplicacao C = A * B.\n");
    }

    printf("\n[OP] C = A^T\n");
    if (matrix_transpose(A, &C) == 0) {
        matrix_print_labeled("C = A^T", C);
        matrix_destroy_labeled("C", C);
        C = NULL;
    } else {
        fprintf(stderr, "Erro na transposicao C = A^T.\n");
    }

    matrix_destroy_labeled("A", A);
    matrix_destroy_labeled("B", B);

    return 0;
}
