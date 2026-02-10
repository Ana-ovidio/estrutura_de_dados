#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

/* ===== Tipos internos (privados ao arquivo) ===== */

typedef struct no {
    int coluna;        
    float valor;
    struct no *prox;
} No;

typedef No* POINT;

struct matrix {
    int linhas;
    int colunas;
    POINT *mat; // vetor de ponteiros (um por linha) 
};

/* ===== Funções auxiliares -> acesso privado (static) ===== */

/**
 * @brief Limpa o buffer do stdin até '\n' ou EOF.
 */
static void clear_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

/**
 * @brief Lê dimensões (linhas, colunas) da entrada padrão.
 */
static void input_tam_matrix(int *linhas, int *colunas) {
    if (!linhas || !colunas) return;
    scanf("%d %d", linhas, colunas);
}

/**
 * @brief Inicializa uma matriz esparsa vazia com dimensões (linhas x colunas).
 *
 * Aloca a estrutura `Matrix` e o vetor de ponteiros de linhas (`m->mat`),
 * inicializando cada posição com NULL (linhas vazias).
 *
 * @param linhas Número de linhas (> 0).
 * @param colunas Número de colunas (> 0).
 *
 * @return Ponteiro para a matriz alocada em caso de sucesso.
 * @return NULL se dimensões forem inválidas ou se falhar alguma alocação.
 *
 * @pre linhas > 0
 * @pre colunas > 0
 * @post A matriz retornada possui todas as linhas vazias (m->mat[i] == NULL).
 */
static Matrix* init_matrix(int linhas, int colunas) {
    if (linhas <= 0 || colunas <= 0) return NULL;

    Matrix *mat = malloc(sizeof *mat);
    if (!mat) return NULL;

    mat->linhas = linhas;
    mat->colunas = colunas;

    mat->mat = malloc((size_t)linhas * sizeof *mat->mat);
    if (!mat->mat) {
        free(mat);
        return NULL;
    }

    for (int i = 0; i < linhas; i++) mat->mat[i] = NULL;
    return mat;
}

/**
 * @brief Lê elementos de uma matriz a partir da entrada padrão e insere na matriz.
 *
 * Solicita repetidamente ao usuário triplas no formato (i, j, valor) e chama
 * `matrix_setelem(m, i, j, valor)` para inserir/atualizar cada elemento.
 * A leitura termina quando o usuário digita i = 0.
 *
 * A função imprime mensagens de apoio usando o rótulo `label` (quando fornecido).
 *
 * @param label Rótulo textual para identificar a matriz nas mensagens (pode ser NULL).
 * @param m Ponteiro para a matriz já inicializada.
 *
 * @return 0 em caso de sucesso.
 * @return 1 se `m` for NULL, se ocorrer erro de leitura (scanf falhar),
 *         ou se `matrix_setelem` retornar erro.
 *
 * @pre m != NULL
 * @post A matriz contém os elementos inseridos (valores 0.0 removem o elemento, via `matrix_setelem`).
 */
static int insert_matrix_stdin(const char *label, Matrix *m) {
    if (!m || !m->mat) return 1;

    printf("\n[%s] Digite elementos no formato: i j valor (0 para terminar)\n",
           label ? label : "Matriz");

    while (1) {
        int i, j;
        float valor;

        printf("[%s] i = ", label ? label : "Matriz");
        if (scanf("%d", &i) != 1) { clear_stdin(); return 1; }
        if (i == 0) break;

        printf("[%s] j valor = ", label ? label : "Matriz");
        if (scanf("%d %f", &j, &valor) != 2) { clear_stdin(); return 1; }

        int err = matrix_setelem(m, i, j, valor);
        if (err) return err;
    }
    return 0;
}

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
 * @return 1 se `m` for NULL, `m->mat` for NULL, ou se falhar a alocação.
 * @return 2 se (i, j) estiver fora dos limites da matriz.
 *
 * @pre m != NULL
 * @pre m->mat != NULL
 * @post A linha permanece ordenada por coluna; se o valor final for 0.0, o elemento é removido.
 */
static int matrix_addelem(Matrix *m, int i, int j, float delta) {
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
        } else {
            atual->valor = nv;
        }
        return 0;
    }

    if (delta == 0.0f) return 0;

    No *novo = malloc(sizeof *novo);
    if (!novo) return 1;

    novo->coluna = j;
    novo->valor = delta;
    novo->prox = atual;

    if (anterior) anterior->prox = novo;
    else m->mat[linha] = novo;

    return 0;
}

/* ===== Implementação da API (matrix.h) ===== */

/**
 * @brief Libera toda a memória associada a uma matriz esparsa.
 *
 * Percorre cada linha da matriz, liberando todos os nós das listas encadeadas.
 * Em seguida libera o vetor `m->mat` e a própria estrutura `m`.
 *
 * @param m Ponteiro para a matriz a ser destruída.
 *
 * @return 0 em caso de sucesso.
 * @return 1 se `m` for NULL.
 *
 * @pre m != NULL
 * @post Toda a memória associada à matriz foi liberada.
 */
int matrix_destroy(Matrix *m) {
    if (!m) return 1;

    if (m->mat) {
        for (int i = 0; i < m->linhas; i++) {
            POINT atual = m->mat[i];
            while (atual) {
                POINT prox = atual->prox;
                free(atual);
                atual = prox;
            }
        }
        free(m->mat);
    }

    free(m);
    return 0;
}

/**
 * @brief Insere, atualiza ou remove um elemento (i, j) na matriz esparsa.
 *
 * Mantém a lista encadeada da linha i ordenada por coluna crescente.
 * - Se já existe um nó na coluna j:
 *   - Se `valor == 0.0`, remove o nó.
 *   - Caso contrário, atualiza o valor do nó.
 * - Se não existe nó na coluna j:
 *   - Se `valor == 0.0`, não faz nada.
 *   - Caso contrário, cria e insere um novo nó na posição correta.
 *
 * Os índices i e j seguem indexação iniciando em 1.
 *
 * @param m Ponteiro para a matriz.
 * @param i Índice da linha (1 ≤ i ≤ m->linhas).
 * @param j Índice da coluna (1 ≤ j ≤ m->colunas).
 * @param valor Valor a ser inserido/atualizado (0.0 remove o elemento).
 *
 * @return 0 em caso de sucesso.
 * @return 1 se `m` for NULL, `m->mat` for NULL, ou se índices fora dos limites,
 *         ou se falhar a alocação.
 *
 * @pre m != NULL
 * @pre m->mat != NULL
 * @post A linha correspondente permanece ordenada por coluna.
 */
int matrix_setelem(Matrix *m, int i, int j, float valor) {
    if (!m || !m->mat) return 1;
    if (i < 1 || i > m->linhas) return 1;
    if (j < 1 || j > m->colunas) return 1;

    int linha = i - 1;
    POINT anterior = NULL;
    POINT atual = m->mat[linha];

    while (atual && atual->coluna < j) {
        anterior = atual;
        atual = atual->prox;
    }

    if (atual && atual->coluna == j) {
        if (valor == 0.0f) {
            if (anterior) anterior->prox = atual->prox;
            else m->mat[linha] = atual->prox;
            free(atual);
        } else {
            atual->valor = valor;
        }
        return 0;
    }

    if (valor == 0.0f) return 0;

    No *novo = malloc(sizeof *novo);
    if (!novo) return 1;

    novo->coluna = j;      /* 1-based */
    novo->valor  = valor;
    novo->prox   = atual;

    if (anterior) anterior->prox = novo;
    else m->mat[linha] = novo;

    return 0;
}

/**
 * @brief Cria uma matriz esparsa lendo dimensões e elementos via entrada padrão.
 *
 * Obtém (linhas, colunas) via `input_tam_matrix`, inicializa a matriz com `init_matrix`
 * e lê os elementos via `insert_matrix_stdin("Matriz", tmp)`. Em caso de erro,
 * destrói a matriz temporária e retorna o código de erro.
 *
 * @param m Endereço de ponteiro que receberá a matriz criada.
 *
 * @return 0 em caso de sucesso.
 * @return 1 se `m` for NULL, se falhar a alocação, ou se ocorrer erro de leitura/inserção.
 *
 * @pre m != NULL
 * @post Em sucesso, `*m` aponta para uma matriz válida; em erro, `*m` permanece NULL.
 */
int matrix_create(Matrix **m) {
    if (!m) return 1;
    *m = NULL;

    int linhas, colunas;
    input_tam_matrix(&linhas, &colunas);

    Matrix *tmp = init_matrix(linhas, colunas);
    if (!tmp) return 1;

    int resultado = insert_matrix_stdin("Matriz", tmp);
    if (resultado) {
        matrix_destroy(tmp);
        return resultado;
    }

    *m = tmp;
    return 0;
}

/**
 * @brief Obtém o valor de um elemento da matriz esparsa.
 *
 * Busca o elemento localizado na posição (x, y) da matriz esparsa.
 * Caso exista um nó correspondente na lista encadeada da linha x,
 * seu valor é retornado em `elem`. Caso contrário, o valor retornado
 * é 0.0.
 *
 * As posições x e y seguem indexação iniciando em 1.
 *
 * @param m Ponteiro constante para a matriz esparsa.
 * @param x Índice da linha (1 ≤ x ≤ m->linhas).
 * @param y Índice da coluna (1 ≤ y ≤ m->colunas).
 * @param elem Ponteiro para armazenar o valor do elemento encontrado.
 *
 * @return 0 em caso de sucesso.
 * @return 1 se a matriz for nula, se `elem` for nulo ou se índices fora dos limites.
 *
 * @pre m != NULL
 * @pre elem != NULL
 * @post `*elem` contém o valor do elemento (x, y) ou 0.0 se não existir.
 */
int matrix_getelem(const Matrix *m, int x, int y, float *elem) {
    if (!m || !elem || !m->mat) return 1;
    if (x < 1 || x > m->linhas) return 1;
    if (y < 1 || y > m->colunas) return 1;

    POINT atual = m->mat[x - 1];

    while (atual && atual->coluna < y) {
        atual = atual->prox;
    }

    if (atual && atual->coluna == y) *elem = atual->valor;
    else *elem = 0.0f;

    return 0;
}

/**
 * @brief Calcula a soma de duas matrizes esparsas de mesmas dimensões.
 *
 * Cria uma nova matriz `res` tal que `res = m + n`.
 *
 * @param m Ponteiro constante para a primeira matriz.
 * @param n Ponteiro constante para a segunda matriz.
 * @param r Endereço de ponteiro que receberá a matriz resultante.
 *
 * @return 0 em caso de sucesso.
 * @return 1 em caso de ponteiros nulos, dimensões incompatíveis ou falha de alocação.
 */
int matrix_add(const Matrix *m, const Matrix *n, Matrix **r) {
    if (!m || !n || !r) return 1;
    if (!m->mat || !n->mat) return 1;
    if (m->linhas != n->linhas || m->colunas != n->colunas) return 1;

    *r = NULL;
    Matrix *res = init_matrix(m->linhas, m->colunas);
    if (!res) return 1;

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
                int check = matrix_setelem(res, i + 1, col, val);
                if (check) {
                    matrix_destroy(res);
                    return check;
                }
            }
        }
    }

    *r = res;
    return 0;
}

/**
 * @brief Calcula a transposta de uma matriz esparsa.
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
 */
int matrix_multiply(const Matrix *m, const Matrix *n, Matrix **r) {
    if (!m || !n || !r) return 1;
    if (!m->mat || !n->mat) return 1;
    if (m->colunas != n->linhas) return 1;

    *r = NULL;
    Matrix *res = init_matrix(m->linhas, n->colunas);
    if (!res) return 1;

    for (int i = 0; i < m->linhas; i++) {
        POINT pm = m->mat[i];

        while (pm) {
            int k = pm->coluna; /* 1-based */
            if (k < 1 || k > n->linhas) {
                matrix_destroy(res);
                return 2;
            }

            POINT pn = n->mat[k - 1];

            while (pn) {
                float prod = pm->valor * pn->valor;
                if (prod != 0.0f) {
                    int check = matrix_addelem(res, i + 1, pn->coluna, prod);
                    if (check) {
                        matrix_destroy(res);
                        return check;
                    }
                }
                pn = pn->prox;
            }
            pm = pm->prox;
        }
    }

    *r = res;
    return 0;
}

/**
 * @brief Imprime matriz no formato:
 *        linhas colunas
 *        i j valor
 *        ...
 *        0
 */
int matrix_print(const Matrix *m) {
    if (!m) return 1;

    printf("%d %d\n", m->linhas, m->colunas);

    for (int i = 0; i < m->linhas; i++) {
        POINT atual = m->mat[i];
        while (atual) {
            printf("%d %d %.1f\n", i + 1, atual->coluna, atual->valor);
            atual = atual->prox;
        }
    }

    printf("0\n\n");
    return 0;
}
