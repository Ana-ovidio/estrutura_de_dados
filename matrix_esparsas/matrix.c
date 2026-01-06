#include <stdio.h>
#include <stdlib.h>

#include "dataclass.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void input_tam_matrix(int *linhas, int *colunas);

int matrix_setelem(Matrix *m, int i, int j, float valor);
int matrix_addelem(Matrix *m, int i, int j, float delta);

int matrix_destroy(Matrix *m);
int matrix_destroy_labeled(const char *label, Matrix *m);

int insert_matrix_stdin(const char *label, Matrix *m);

Matrix* init_matrix(int linhas, int colunas);

int matrix_create(Matrix **m);
int matrix_create_labeled(const char *label, Matrix **m);

int matrix_getelem(const Matrix *m, int x, int y, float *elem);

int matrix_add(const Matrix *m, const Matrix *n, Matrix **r);
int matrix_transpose(const Matrix *m, Matrix **r);
int matrix_multiply(const Matrix *m, const Matrix *n, Matrix **r);

void matrix_print(const Matrix *m);
void matrix_print_labeled(const char *label, const Matrix *m);

//helper
void clear_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
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
 * @return 1 se `m` for NULL, se ocorrer erro de leitura de `i` (scanf falhar),
 *         ou se `matrix_setelem` retornar erro genérico.
 * @return 2 se ocorrer erro de leitura de `j` e `valor` (scanf falhar).
 *
 * @pre m != NULL
 * @post A matriz contém os elementos inseridos (valores 0.0 removem o elemento, via `matrix_setelem`).
 */
int insert_matrix_stdin(const char *label, Matrix *m) {
    if (!m || !m->mat) return 1;

    printf("\n[%s] Digite elementos no formato: i j valor (0 para terminar)\n",
           label ? label : "Matriz");

    int run = 1;
    while (run) {
        int i, j;
        float valor;

        printf("[%s] i = ", label ? label : "Matriz");
        if (scanf("%d", &i) != 1) {
            clear_stdin();
            return 1;
        }

        if (i == 0) break;

        printf("[%s] j valor = ", label ? label : "Matriz");
        if (scanf("%d %f", &j, &valor) != 2) {
            clear_stdin();
            return 1;
        }

        int error = matrix_setelem(m, i, j, valor);
        if (error) {
            fprintf(stderr,
                    "[%s] Erro ao inserir (%d, %d, %.2f). codigo=%d\n",
                    label ? label : "Matriz", i, j, valor, error);
            return error;
        }

    }
    return 0;
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
Matrix* init_matrix(int linhas, int colunas) {
    if (linhas <= 0 || colunas <= 0) return NULL;

    Matrix *mat = (Matrix*)malloc(sizeof(Matrix));
    if (!mat) return NULL;

    mat->linhas = linhas;
    mat->colunas = colunas;

    mat->mat = (POINT*)malloc((float)linhas * sizeof(POINT));
    if (!mat->mat) {
        free(mat);
        return NULL;
    }

    for (int i = 0; i < linhas; i++) mat->mat[i] = NULL;
    return mat;
}

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
 * @brief Libera a matriz esparsa exibindo logs com um rótulo.
 *
 * Imprime uma mensagem informando que a matriz será liberada e em seguida chama
 * `matrix_destroy(m)`. Se `m` for NULL, imprime log específico e retorna erro.
 *
 * @param label Rótulo textual para identificar a matriz no log (pode ser NULL).
 * @param m Ponteiro para a matriz a ser destruída.
 *
 * @return 0 se a matriz foi destruída com sucesso.
 * @return 1 se `m` for NULL.
 *
 * @pre label pode ser NULL
 * @post Se `m` não era NULL, a matriz foi destruída.
 */
int matrix_destroy_labeled(const char *label, Matrix *m) {
    if (!m) {
        printf("[DESTROY %s] (NULL)\n", label ? label : "");
        return 1;
    }

    printf("[DESTROY %s] liberando matriz (%d x %d)\n",
           label ? label : "", m->linhas, m->colunas);
    return matrix_destroy(m);
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
 * @return 0 em caso de sucesso (inclui remoção bem-sucedida ou valor 0 ignorado).
 * @return 1 se `m` for NULL, `m->mat` for NULL, ou se falhar a alocação de um novo nó.
 * @return 2 se (i, j) estiver fora dos limites da matriz.
 *
 * @pre m != NULL
 * @pre m->mat != NULL
 * @post A linha correspondente permanece ordenada por coluna.
 */
int matrix_setelem(Matrix *m, int i, int j, float valor) {
    if (!m || !m->mat) return 1;
    if (i < 1 || i > m->linhas) return 1;
    if (j < 1 || j > m->colunas) return 1;

    // Inicia-se com 1, conforme o enunciado
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

    No *novo = (No*)malloc(sizeof(No));
    if (!novo) return 1;

    novo->coluna = j;
    novo->valor = valor;
    novo->prox = atual;

    if (anterior) anterior->prox = novo;
    else m->mat[linha] = novo;

    return 0;
}


/**
 * @brief Cria uma matriz esparsa lendo dimensões e elementos via entrada padrão.
 *
 * Obtém (linhas, colunas) via `input_tam_matrix`, inicializa a matriz com `init_matrix`
 * e lê os elementos via `insert_matrix_stdin("Matriz", tmp)`. Em caso de erro durante
 * a leitura/inserção, destrói a matriz temporária e retorna o código de erro.
 *
 * @param m Endereço de ponteiro que receberá a matriz criada.
 *
 * @return 0 em caso de sucesso.
 * @return 1 se `m` for NULL, se falhar a alocação, ou se `input_tam_matrix` fornecer dimensões inválidas.
 * @return Código de erro propagado por `insert_matrix_stdin` (ex.: 2).
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
 * @brief Cria uma matriz esparsa com logs identificados por rótulo.
 *
 * Lê dimensões via `input_tam_matrix`, inicializa a matriz e lê os elementos com
 * `insert_matrix_stdin(label, tmp)`, imprimindo mensagens com o rótulo fornecido.
 * Em caso de erro, destrói a matriz temporária e retorna o código correspondente.
 *
 * @param label Rótulo textual para identificar a matriz nas mensagens (pode ser NULL).
 * @param m Endereço de ponteiro que receberá a matriz criada.
 *
 * @return 0 em caso de sucesso.
 * @return 1 se `m` for NULL ou se falhar a alocação.
 * @return Código de erro propagado por `insert_matrix_stdin` (ex.: 2).
 *
 * @pre m != NULL
 * @post Em sucesso, `*m` aponta para uma matriz válida; em erro, `*m` permanece NULL.
 */
int matrix_create_labeled(const char *label, Matrix **m) {
    *m = NULL;

    int linhas, colunas;
    printf("\n[%s] Digite dimensoes: linhas colunas\n", label ? label : "Matriz");
    fflush(stdout);

    input_tam_matrix(&linhas, &colunas);

    Matrix *tmp = init_matrix(linhas, colunas);
    if (!tmp) return 1;

    printf("[%s] criando matriz (%d x %d)\n", label ? label : "Matriz", linhas, colunas);

    int resultado = insert_matrix_stdin(label, tmp);
    if (resultado) {
        matrix_destroy(tmp);
        return resultado;
    }

    *m = tmp;
    return 0;
}

void input_tam_matrix(int *linhas, int *colunas) {
    if (!linhas || !colunas) return;
    scanf("%d %d", linhas, colunas);
}


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

void matrix_print(const Matrix *m) {
    if (!m) return;

    printf("\nMatriz esparsa (%d x %d)\n", m->linhas, m->colunas);

    for (int i = 0; i < m->linhas; i++) {
        printf("Linha %d:", i + 1);

        POINT atual = m->mat[i];
        if (!atual) printf(" [vazia]");

        while (atual) {
            printf(" (%d, %.2f)", atual->coluna, atual->valor);
            atual = atual->prox;
        }
        printf("\n");
    }
}

void matrix_print_labeled(const char *label, const Matrix *m) {
    if (label) printf("\n=== %s ===\n", label);
    matrix_print(m);
}


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
}
