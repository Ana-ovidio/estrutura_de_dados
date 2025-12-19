#include "tooling.h"
#include "index_utils.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/**
 * @brief Remove caracteres de nova linha do final de uma string.
 *
 * Elimina caracteres '\n' e '\r' ao final da string, modificando-a
 * diretamente. Usado para tratar linhas lidas de arquivos texto.
 *
 * @param s String a ser processada.
 */
void trim_newline(char *s) {
    int tamanho = strlen(s);
    while (tamanho > 0 && (s[tamanho-1] == '\n' || s[tamanho-1] == '\r')) {
        s[tamanho-1] = '\0';
        tamanho--;
    }
}

/**
 * @brief Normaliza uma string ASCII.
 *
 * Converte todos os caracteres para minúsculas e remove espaços
 * em branco no início e no final da string. A normalização garante
 * consistência na comparação de palavras-chave.
 *
 * @param s String a ser normalizada.
 */
void normalize_ascii(char *s) {
    // deixa em minúsculo (ASCII) e remove espaços nas pontas
    // (a remoção de pontuação no texto é feita no tokenizador)
    // memmove = tirar espaços do começo com + 1 para levar em conta '\0'
    int i = 0;
    while (s[i] && isspace((char)s[i])) i++;
    if (i) memmove(s, s + i, strlen(s + i) + 1); // tirar espaços do começo 

    // trim direita
    int n = strlen(s);
    while (n > 0 && isspace((char)s[n-1])) {
        s[n-1] = '\0';
        n--;
    }

    for (int k = 0; s[k]; k++) {
        s[k] = (char)tolower((char)s[k]);
    }
}

/**
 * @brief Insere uma palavra-chave normalizada no índice remissivo.
 *
 * Caso a palavra já exista no índice, nenhuma modificação é realizada.
 * Caso contrário, uma nova entrada é criada e inserida no bucket
 * correspondente da tabela hash.
 *
 * @param idx Índice remissivo.
 * @param key_norm Palavra-chave normalizada.
 *
 * @return 0 em caso de sucesso.
 */
int insere_index(Index *idx, const char *key_norm) {
    int bucket = 0;
    if (busca_index(idx, key_norm, &bucket)) {
        return 0;
    }

    Entrada *entrada = (Entrada *)malloc(sizeof(Entrada));
    strncpy(entrada->key, key_norm, KEY_MAX); // copiar uma string para outra
    entrada->key[KEY_MAX] = '\0';
    entrada->linhas = NULL;
    entrada->count_linhas = 0;
    entrada->cap = 0;

    entrada->prox = idx->buckets[bucket];
    idx->buckets[bucket] = entrada;
    return 0;
}

/**
 * @brief Registra uma nova ocorrência de linha para uma palavra-chave.
 *
 * Adiciona o número da linha ao vetor de ocorrências associado à
 * palavra-chave. Caso o vetor esteja cheio, sua capacidade é
 * automaticamente expandida.
 *
 * @param e Entrada do índice correspondente à palavra-chave.
 * @param linhas_no Número da linha onde a palavra ocorre.
 */
void add_line(Entrada *e, int linhas_no) {
    if (e->count_linhas == e->cap) {
        int novo_cap = (e->cap == 0) ? 4 : e->cap * 2; // capacidade inicial 4
        int *tmp = (int *)realloc(e->linhas, novo_cap * sizeof(int));

        e->linhas = tmp;
        e->cap = novo_cap;
    }
    int count = e->count_linhas++;
    e->linhas[count] = linhas_no;
}

/**
 * @brief Analisa uma linha de texto e registra ocorrências no índice.
 *
 * Percorre a linha caractere a caractere, identificando tokens formados
 * por letras, dígitos ou sublinhado ('_'). Para cada token identificado,
 * verifica se ele pertence ao índice remissivo e, em caso afirmativo,
 * registra a linha de ocorrência.
 *
 * @param idx Índice remissivo.
 * @param line Linha de texto a ser analisada.
 * @param line_no Número da linha no arquivo de texto.
 */
void analisa_tokens(Index *idx, char *line, int line_no) {
    // tokeniza: letras/dígitos/_ contam como palavra; resto separa
    char tok[KEY_MAX + 1];
    int t = 0;

    for (int i = 0;; i++) {
        char c = (char)line[i];

        int check_palavra = (c != 0) && (isalnum(c) || c == '_');

        if (check_palavra) {
            if (t < KEY_MAX) {
                tok[t++] = (char)tolower(c);
            }
        }

        if (!check_palavra) {
            if (t > 0) {
                tok[t] = '\0';
                Entrada *entrada = busca_index(idx, tok, NULL);
                if (entrada) {
                    add_line(entrada, line_no);
                }
                t = 0;
            }
            if (c == 0) break;
        }
    }
}
