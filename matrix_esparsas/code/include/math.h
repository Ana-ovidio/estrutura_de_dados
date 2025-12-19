#ifndef MATH_H
#define MATH_H

#include "dataclass.h"

int matrix_add(const Matrix *m, const Matrix *n, Matrix **r);
int matrix_multiply(const Matrix *m, const Matrix *n, Matrix **r);
int matrix_transpose(const Matrix *m, Matrix **r);

#endif
