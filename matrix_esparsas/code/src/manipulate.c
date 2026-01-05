#include "manipulate.h"
#include <math.h>

/**
 * @brief Obtém o valor de um elemento da matriz esparsa.
 *
 * Busca o elemento localizado na posição (x, y) da matriz esparsa.
 * Caso exista um nó correspondente na lista encadeada da linha x,
 * seu valor é retornado em `elem`. Caso contrário, o valor retornado
 * é 0.0, representando um elemento nulo da matriz.
 *
 * As posições x e y seguem indexação iniciando em 1.
 *
 * @param m Ponteiro constante para a matriz esparsa.
 * @param x Índice da linha (1 ≤ x ≤ m->linhas).
 * @param y Índice da coluna (1 ≤ y ≤ m->colunas).
 * @param elem Ponteiro para armazenar o valor do elemento encontrado.
 *
 * @return 0 em caso de sucesso.
 * @return 1 se a matriz for nula, se `elem` for nulo ou se os índices
 *         estiverem fora dos limites da matriz.
 *
 * @pre m != NULL
 * @pre elem != NULL
 * @post `*elem` contém o valor do elemento (x, y) ou 0.0 se o elemento
 *       não estiver explicitamente armazenado.
 */

 int matrix_getelem(const Matrix *m, int x, int y, float *elem) {
    if (!m || !elem || !m->mat) return 1;
    if (x < 1 || x > m->linhas) return 1;
    if (y < 1 || y > m->colunas) return 1;

    POINT atual = m->mat[x - 1];

    while (atual && atual->coluna < y) {
        atual = atual->prox;
    }

    if (atual && atual->coluna == y) {
        *elem = atual->valor;
    } else {
        *elem = 0.0f;
    }

    return 0;
}
