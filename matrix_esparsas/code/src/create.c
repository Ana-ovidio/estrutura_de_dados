#include <stdio.h>
#include <stdlib.h>
#include "create.h"
#include "inputs.h"
#include <math.h>

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
