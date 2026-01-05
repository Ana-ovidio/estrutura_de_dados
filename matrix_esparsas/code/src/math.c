#include <stdlib.h>
#include "math.h"
#include "create.h"

/**
 * @brief Soma um incremento (delta) ao elemento (i, j) de uma matriz esparsa.
 *
 * Procura o elemento na posição (i, j) na lista encadeada da linha i (ordenada por coluna).
 * - Se o elemento existir, soma `delta` ao valor atual.
 *   - Se o novo valor resultar em 0.0, o nó é removido da lista.
 * - Se o elemento não existir, um novo nó é criado com valor `delta` (desde que `delta != 0.0`).
 *
 * Os índices i e j seguem indexação iniciando em 1.
 *
 * @param m Ponteiro para a matriz esparsa (modificada in-place).
 * @param i Índice da linha (1 ≤ i ≤ m->linhas).
 * @param j Índice da coluna (1 ≤ j ≤ m->colunas).
 * @param delta Incremento a ser somado (0.0 não altera a matriz).
 *
 * @return 0 em caso de sucesso (inclui delta 0 ignorado e remoção bem-sucedida).
 * @return 1 se `m` for NULL, `m->mat` for NULL, ou se falhar a alocação de um novo nó.
 * @return 2 se (i, j) estiver fora dos limites da matriz.
 *
 * @pre m != NULL
 * @pre m->mat != NULL
 * @post A linha correspondente permanece ordenada por coluna; se o valor final for 0.0, o elemento é removido.
 */
int matrix_addelem(Matrix *m, int i, int j, float delta) {
    if (!m || !m->mat) return 1;
    if (i < 1 || i > m->linhas) return 2;
    if (j < 1 || j > m->colunas) return 2;

    int linha = i - 1;
    POINT anterior = NULL;
    POINT atual = m->mat[linha];

    while (atual && atual->coluna < j) {
        anterior = atual;
        atual = atual->prox;
    }

    if (atual && atual->coluna == j) {
        float nv = atual->valor + delta;

        if (nv == 0.0f) {
            if (anterior) anterior->prox = atual->prox;
            else m->mat[linha] = atual->prox;
            free(atual);
            return 0;
        }

        atual->valor = nv;
        return 0;
    }

    No *novo = (No*)malloc(sizeof(No));
    if (!novo) return 1;

    novo->coluna = j;
    novo->valor = delta;
    novo->prox = atual;

    if (anterior) anterior->prox = novo;
    else m->mat[linha] = novo;

    return 0;
}


/**
 * @brief Calcula a soma de duas matrizes esparsas de mesmas dimensões.
 *
 * Cria uma nova matriz `res` tal que `res = m + n`. A soma é feita linha a linha,
 * percorrendo simultaneamente as listas encadeadas (ordenadas por coluna) de `m` e `n`,
 * garantindo complexidade proporcional ao número de elementos não nulos.
 *
 * Elementos cujo resultado seja 0.0 não são armazenados na matriz resultante.
 *
 * @param m Ponteiro constante para a primeira matriz (operando).
 * @param n Ponteiro constante para a segunda matriz (operando).
 * @param r Endereço de ponteiro que receberá a matriz resultante.
 *
 * @return 0 em caso de sucesso.
 * @return 1 se `r` for NULL, se `m`/`n` forem NULL, se as dimensões forem incompatíveis,
 *         ou se falhar alguma alocação.
 * @return Código de erro propagado por `matrix_setelem`.
 *
 * @pre m != NULL
 * @pre n != NULL
 * @pre r != NULL
 * @pre m->linhas == n->linhas e m->colunas == n->colunas
 * @post Em sucesso, `*r` aponta para uma nova matriz alocada; em erro, `*r` permanece NULL.
 */
int matrix_add(const Matrix *m, const Matrix *n, Matrix **r) {
    if (!m || !n || !r) return 1;
    if (!m->mat || !n->mat) return 1;
    if (m->linhas != n->linhas || m->colunas != n->colunas) return 1;

    *r = NULL;
    Matrix *matrix_resultado = init_matrix(m->linhas, m->colunas);
    if (!matrix_resultado) return 1;

    for (int i = 0; i < m->linhas; i++) {
        POINT pm = m->mat[i];
        POINT pn = n->mat[i];

        while (pm || pn) {
            int col;
            float val;

            if (!pn || (pm && pm->coluna < pn->coluna)) {
                col = pm->coluna;
                val = pm->valor;
                pm = pm->prox;
            } else if (!pm || pn->coluna < pm->coluna) {
                col = pn->coluna;
                val = pn->valor;
                pn = pn->prox;
            } else {
                col = pm->coluna;
                val = pm->valor + pn->valor;
                pm = pm->prox;
                pn = pn->prox;
            }

            if (val != 0.0f) {
                int check = matrix_setelem(matrix_resultado, i + 1, col, val);
                if (check) {
                    matrix_destroy(matrix_resultado);
                    return check;
                }
            }
        }
    }

    *r = matrix_resultado;
    return 0;
}

/**
 * @brief Calcula a transposta de uma matriz esparsa.
 *
 * Cria uma nova matriz `res` com dimensões (m->colunas x m->linhas) tal que
 * `res(j, i) = m(i, j)` para todo elemento não nulo armazenado em `m`.
 *
 * @param m Ponteiro constante para a matriz de entrada.
 * @param r Endereço de ponteiro que receberá a matriz transposta.
 *
 * @return 0 em caso de sucesso.
 * @return 1 se `r` for NULL, se `m` for NULL, ou se falhar alguma alocação.
 * @return Código de erro propagado por `matrix_setelem`.
 *
 * @pre m != NULL
 * @pre r != NULL
 * @post Em sucesso, `*r` aponta para a transposta de `m`; em erro, `*r` permanece NULL.
 */
int matrix_transpose(const Matrix *m, Matrix **r) {
    if (!m || !m->mat || !r) return 1;

    *r = NULL;
    Matrix *res = init_matrix(m->colunas, m->linhas);
    if (!res) return 1;

    for (int i = 0; i < m->linhas; i++) {
        POINT atual = m->mat[i];
        while (atual) {
            
            int rr = matrix_setelem(res, atual->coluna, i + 1, atual->valor);
            if (rr) {
                matrix_destroy(res);
                return rr;
            }
            atual = atual->prox;
        }
    }

    *r = res;
    return 0;
}

/**
 * @brief Calcula o produto de duas matrizes esparsas.
 *
 * Cria uma nova matriz `res` tal que `res = m * n`. O algoritmo percorre cada linha de `m` e,
 * para cada elemento não nulo m(i, k), percorre a linha k de `n` (considerando o acesso por linhas),
 * acumulando contribuições em `res(i, j)` via somas incrementais.
 *
 * Elementos cujo valor acumulado final resulte em 0.0 são removidos (não armazenados).
 *
 * @param m Ponteiro constante para a matriz à esquerda (m x p).
 * @param n Ponteiro constante para a matriz à direita (p x q).
 * @param r Endereço de ponteiro que receberá a matriz produto (m x q).
 *
 * @return 0 em caso de sucesso.
 * @return 1 se `r` for NULL, se `m`/`n` forem NULL, se as dimensões forem incompatíveis
 *         (m->colunas != n->linhas), ou se falhar alguma alocação.
 * @return Código de erro propagado por `matrix_addelem`.
 *
 * @pre m != NULL
 * @pre n != NULL
 * @pre r != NULL
 * @pre m->colunas == n->linhas
 * @post Em sucesso, `*r` aponta para uma nova matriz alocada com o produto; em erro, `*r` permanece NULL.
 */
int matrix_multiply(const Matrix *m, const Matrix *n, Matrix **r) {
    if (!m || !n || !r) return 1;
    if (!m->mat || !n->mat) return 1;
    if (m->colunas != n->linhas) return 1;

    *r = NULL;
    Matrix *matrix_resultado = init_matrix(m->linhas, n->colunas);
    if (!matrix_resultado) return 1;

    for (int i = 0; i < m->linhas; i++) {
        POINT pm = m->mat[i];

        while (pm) {
            int k = pm->coluna;
            if (k < 1 || k > n->linhas) {
                matrix_destroy(matrix_resultado);
                return 2;
            }

            POINT pn = n->mat[k - 1];

            while (pn) {
                float prod = pm->valor * pn->valor;
                if (prod != 0.0) {
                    int check = matrix_addelem(matrix_resultado,
                                               i + 1,
                                               pn->coluna,
                                               prod);
                    if (check) {
                        matrix_destroy(matrix_resultado);
                        return check;
                    }
                }
                pn = pn->prox;
            }
            pm = pm->prox;
        }
    }

    *r = matrix_resultado;
    return 0;
}
