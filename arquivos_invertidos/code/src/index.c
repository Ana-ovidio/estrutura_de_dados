#include "etl.h"
#include "tooling.h"
#include "index_utils.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Programa principal para consulta a um índice remissivo.
 *
 * O programa recebe dois arquivos como parâmetros de linha de comando:
 *  - um arquivo contendo palavras-chave (keys)
 *  - um arquivo de texto a ser indexado
 *
 * A partir desses arquivos, o índice remissivo é criado e o usuário
 * pode consultar uma palavra-chave, obtendo as linhas em que ela ocorre
 * no texto.
 *
 * @param argc Número de argumentos de linha de comando.
 * @param argv Vetor de argumentos de linha de comando.
 *
 * @return 0 em caso de execução bem-sucedida.
 * @return Valor diferente de zero em caso de erro.
 */
int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Erro: numero insuficiente de parametros.\n");
        fprintf(stderr, "Sintaxe: %s key_file_name txt_file_name\n", argv[0]);
        return 1;
    }

    Index *idx = NULL;

    /**
     * Cria o índice remissivo a partir dos arquivos informados.
     */
    if (index_createfrom(argv[1], argv[2], &idx) != 0) {
        fprintf(stderr, "Erro: criacao do indice\n");
        return 1;
    }

    char keyword[17];
    printf("Qual a palavra-chave a procurar?\n");

    /**
     * Lê a palavra-chave digitada pelo usuário.
     * O formato %16[^\n] garante que não haverá estouro de buffer.
     */
    if (scanf(" %16[^\n]", keyword) != 1) {
        fprintf(stderr, "Erro ao ler palavra-chave\n");
        index_destroy(&idx);
        return 1;
    }

    int *occurrences = NULL;
    int n_occurrences = 0;

    /**
     * Consulta o índice remissivo em busca da palavra-chave informada.
     */
    if (index_get(idx, keyword, &occurrences, &n_occurrences) != 0) {
        fprintf(stderr, "Erro: palavra nao pertence ao indice\n");
    } else {
        if (n_occurrences <= 0) {
            printf("Nao ha ocorrencias de %s\n", keyword);
        } else {
            printf("%d ocorrencias de %s: ", n_occurrences, keyword);
            for (int i = 0; i < n_occurrences; i++) {
                printf("%d", occurrences[i]);
                if (i < n_occurrences - 1)
                    printf(", ");
            }
            printf("\n");
        }
        free(occurrences);
    }

    /**
     * Libera toda a memória associada ao índice remissivo.
     */
    index_destroy(&idx);
    return 0;
}
