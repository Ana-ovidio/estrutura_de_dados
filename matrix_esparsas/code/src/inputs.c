#include <stdio.h>
#include "inputs.h"
#include <math.h>

void input_tam_matrix(int *linhas, int *colunas) {
    if (!linhas || !colunas) return;
    scanf("%d %d", linhas, colunas);
}
