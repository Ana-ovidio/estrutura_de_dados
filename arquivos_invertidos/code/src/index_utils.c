#include "index_utils.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Libera a memória associada a uma única entrada do índice.
 *
 * Libera o vetor de linhas associado à palavra-chave e,
 * em seguida, libera a própria estrutura Entrada.
 *
 * @param e Ponteiro para a entrada a ser liberada.
 */
static void acesso_free(Entrada *e) {
    if (!e) return;
    free(e->linhas);
    free(e);
}

/**
 * @brief Libera toda a memória associada a um índice remissivo.
 *
 * Percorre todos os buckets do índice, liberando cada lista
 * encadeada de entradas. Ao final, libera o vetor de buckets
 * e a estrutura principal do índice.
 *
 * @param idx Ponteiro para o ponteiro do índice a ser destruído.
 *
 * @return 0 em caso de sucesso.
 * @return Valor diferente de zero se o índice for inválido.
 *
 * @post O ponteiro *idx passa a valer NULL.
 */
int index_destroy(Index **idx) {
    if (idx == NULL || *idx == NULL) return 1;

    Index *p = *idx;

    for (int bucket = 0; bucket < p->num_buckets; bucket++) {
        Entrada *atual = p->buckets[bucket];
        while (atual != NULL) {
            Entrada *prox = atual->prox;
            acesso_free(atual);
            atual = prox;
        }
        p->buckets[bucket] = NULL;
    }

    free(p->buckets);
    free(p);
    *idx = NULL;
    return 0;
}

/**
 * @brief Cria um índice remissivo vazio.
 *
 * Aloca dinamicamente a estrutura Index e inicializa
 * um vetor de buckets, onde cada bucket representa o
 * início de uma lista encadeada de entradas.
 *
 * @param num_buckets Número de buckets a serem utilizados no índice.
 *
 * @return Ponteiro para o índice criado.
 * @return NULL se ocorrer erro de alocação ou se num_buckets <= 0.
 */
Index *index_create_empty(int num_buckets) {
    if (num_buckets <= 0) return NULL;

    Index *idx = malloc(sizeof(Index));
    if (!idx) return NULL;

    idx->num_buckets = num_buckets;
    idx->buckets = malloc(num_buckets * sizeof(Entrada *));
    if (!idx->buckets) {
        free(idx);
        return NULL;
    }

    for (int i = 0; i < num_buckets; i++)
        idx->buckets[i] = NULL;

    return idx;
}


/**
 * @brief Função de hash DJB2 para strings.
 *
 * Calcula um valor de hash a partir de uma string utilizando
 * o algoritmo DJB2, amplamente empregado em tabelas hash
 * devido à sua simplicidade e boa distribuição.
 *
 * @param s String a ser transformada em valor de hash.
 *
 * @return Valor de hash calculado.
 */
unsigned long hash_djb2(const char *s) {
    unsigned long h = 5381;
    unsigned char c;

    while ((c = (unsigned char)*s++)) {
        h = (h * 33u) + c;
    }
    return h;
}

/**
 * @brief Busca uma palavra-chave no índice remissivo.
 *
 * Calcula o bucket correspondente à palavra-chave normalizada
 * e percorre a lista encadeada associada, procurando uma
 * entrada cujo campo key seja igual à palavra buscada.
 *
 * @param idx Índice remissivo.
 * @param key_norm Palavra-chave normalizada (minúsculas, sem espaços).
 * @param bucket_out Ponteiro opcional para armazenar o bucket calculado.
 *
 * @return Ponteiro para a entrada encontrada.
 * @return NULL se a palavra não estiver presente no índice.
 */
Entrada *busca_index(Index *idx, const char *key_norm, int *bucket_out) {
    if (!idx || !key_norm || idx->num_buckets <= 0) return NULL;

    unsigned long h = hash_djb2(key_norm);
    int b = (int)(h % (unsigned long)idx->num_buckets);

    if (b < 0 || b >= idx->num_buckets) return NULL;

    if (bucket_out) *bucket_out = b;

    Entrada *atual = idx->buckets[b];
    while (atual) {
        if (strcmp(atual->key, key_norm) == 0)
            return atual;
        atual = atual->prox;
    }
    return NULL;
}

