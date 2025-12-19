#include "etl.h"
#include "tooling.h"
#include "index_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINEBUF 4096
#define HASH_PRIMO 1009

/**
 * @brief Cria um índice remissivo a partir de dois arquivos texto.
 *
 * A função constrói um índice remissivo em duas etapas:
 *  1) Lê o arquivo de palavras-chave (`key_file`) e inicializa o índice
 *     apenas com essas palavras, sem ocorrências associadas.
 *  2) Varre o arquivo de texto (`text_file`) linha a linha e registra,
 *     para cada palavra-chave previamente cadastrada, as linhas em que
 *     ela aparece.
 *
 * @param key_file Caminho para o arquivo contendo as palavras-chave.
 * @param text_file Caminho para o arquivo de texto a ser indexado.
 * @param idx Ponteiro para o índice criado.
 *
 * @return 0 em caso de sucesso.
 * @return Valor diferente de zero em caso de erro (falha de alocação,
 *         erro ao abrir arquivos, etc.).
 */
int index_createfrom(const char *key_file, const char *text_file, Index **idx) {
    if (!idx) return 1;
    *idx = NULL;

    Index *out = index_create_empty(HASH_PRIMO);
    if (!out) return 2;

    FILE *fk = fopen(key_file, "r");
    if (!fk) {
        index_destroy(&out);
        return 3;
    }

    char buf[LINEBUF];

    /* 1) lê palavras-chave */
    while (fgets(buf, sizeof(buf), fk)) {
        trim_newline(buf);
        normalize_ascii(buf);
        if (buf[0] == '\0') continue;

        if (insere_index(out, buf) != 0) {
            fclose(fk);
            index_destroy(&out);
            return 4;
        }
    }
    fclose(fk);

    /* 2) varre texto e registra linhas */
    FILE *ft = fopen(text_file, "r");
    if (!ft) {
        index_destroy(&out);
        return 5;
    }

    int line_no = 0;
    while (fgets(buf, sizeof(buf), ft)) {
        line_no++;
        analisa_tokens(out, buf, line_no);
    }
    fclose(ft);

    *idx = out;
    return 0;
}

/**
 * @brief Recupera as ocorrências de uma palavra-chave no índice.
 *
 * Procura a palavra-chave no índice remissivo e, se encontrada,
 * aloca dinamicamente um vetor contendo todas as linhas em que
 * a palavra ocorre.
 *
 * @param idx Índice remissivo.
 * @param key Palavra-chave a ser buscada.
 * @param occurrences Ponteiro para o vetor de ocorrências (alocado pela função).
 * @param num_occurrences Número de ocorrências encontradas.
 *
 * @return 0 em caso de sucesso.
 * @return Valor diferente de zero se a palavra não existir no índice
 *         ou se ocorrer erro de alocação.
 *
 * @note O vetor retornado em `occurrences` deve ser liberado pelo chamador.
 */
int index_get(
    const Index *idx,
    const char *key,
    int **occurrences,
    int *num_occurrences
) {
    if (!idx || !key || !occurrences || !num_occurrences)
        return 1;

    *occurrences = NULL;
    *num_occurrences = 0;

    char key_norm[KEY_MAX + 1];
    strncpy(key_norm, key, KEY_MAX);
    key_norm[KEY_MAX] = '\0';
    normalize_ascii(key_norm);

    Entrada *e = busca_index((Index *)idx, key_norm, NULL);
    if (!e)
        return 2;

    if (e->count_linhas <= 0)
        return 0;

    int *v = malloc(e->count_linhas * sizeof(int));
    if (!v)
        return 3;

    for (int i = 0; i < e->count_linhas; i++)
        v[i] = e->linhas[i];

    *occurrences = v;
    *num_occurrences = e->count_linhas;
    return 0;
}

/**
 * @brief Função auxiliar de comparação para ordenação alfabética.
 *
 * Utilizada pela função qsort para ordenar as entradas do índice
 * de acordo com a palavra-chave.
 *
 * @param a Ponteiro para o primeiro elemento.
 * @param b Ponteiro para o segundo elemento.
 *
 * @return Valor negativo, zero ou positivo conforme a ordem lexicográfica.
 */
static int cmp_entrada(const void *a, const void *b) {
    const Entrada *ea = *(const Entrada **)a;
    const Entrada *eb = *(const Entrada **)b;
    return strcmp(ea->key, eb->key);
}

/**
 * @brief Imprime o índice remissivo completo em ordem alfabética.
 *
 * Percorre todos os buckets do índice, coleta as entradas existentes,
 * ordena-as alfabeticamente e imprime cada palavra-chave seguida das
 * linhas em que ocorre.
 *
 * @param idx Índice remissivo a ser impresso.
 *
 * @return 0 em caso de sucesso.
 * @return Valor diferente de zero em caso de erro.
 */
int index_print(const Index *idx) {
    if (!idx) return 1;

    int total = 0;
    for (int b = 0; b < idx->num_buckets; b++)
        for (Entrada *e = idx->buckets[b]; e; e = e->prox)
            total++;

    if (total == 0)
        return 0;

    Entrada **vet = malloc(total * sizeof(Entrada *));
    if (!vet)
        return 2;

    int k = 0;
    for (int b = 0; b < idx->num_buckets; b++)
        for (Entrada *e = idx->buckets[b]; e; e = e->prox)
            vet[k++] = e;

    qsort(vet, total, sizeof(Entrada *), cmp_entrada);

    for (int i = 0; i < total; i++) {
        Entrada *e = vet[i];
        printf("%s:", e->key);
        for (int j = 0; j < e->count_linhas; j++)
            printf(" %d", e->linhas[j]);
        printf("\n");
    }

    free(vet);
    return 0;
}
