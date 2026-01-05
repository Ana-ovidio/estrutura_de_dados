#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "etl.h"
#include "index_utils.h"

#define ASSERT(cond, msg)                        \
    do {                                        \
        if (!(cond)) {                          \
            fprintf(stderr, "[FAIL] %s\n", msg);\
            return 1;                           \
        }                                       \
    } while (0)

static int assert_occurrences(Index *idx,
                              const char *word,
                              const int *expected,
                              int expected_n) {
    int *occ = NULL;
    int n = 0;

    if (index_get(idx, word, &occ, &n) != 0) {
        return 1;
    }

    if (n != expected_n) {
        free(occ);
        return 1;
    }

    for (int i = 0; i < n; i++) {
        if (occ[i] != expected[i]) {
            free(occ);
            return 1;
        }
    }

    free(occ);
    return 0;
}

int main(void) {
    Index *idx = NULL;

    /* ---------- TESTE: criação do índice ---------- */
    ASSERT(
        index_createfrom(
            "data/keys_test.txt",
            "data/texto_test.txt",
            &idx
        ) == 0,
        "Falha ao criar indice"
    );

    /* ---------- TESTE: palavra existente ---------- */
    {
        int esperado[] = {1, 4};
        ASSERT(
            assert_occurrences(idx, "casa", esperado, 2) == 0,
            "Erro ocorrencias palavra 'casa'"
        );
    }

    /* ---------- TESTE: palavra existente ---------- */
    {
        int esperado[] = {1, 3};
        {
            int *occ = NULL;
            int n = 0;
            index_get(idx, "texto", &occ, &n);
        
            printf("[DEBUG] ocorrencias de 'texto': n=%d -> ", n);
            for (int i = 0; i < n; i++) {
                printf("%d ", occ[i]);
            }
            printf("\n");
        
            free(occ);
        }
        ASSERT(
            assert_occurrences(idx, "texto", esperado, 2) == 0,
            "Erro ocorrencias palavra 'texto'"
        );
    }

    /* ---------- TESTE: palavra existente ---------- */
    {
        int esperado[] = {3};
        ASSERT(
            assert_occurrences(idx, "indice", esperado, 1) == 0,
            "Erro ocorrencias palavra 'indice'"
        );
    }

    /* ---------- TESTE: palavra inexistente ---------- */
    {
        int *occ = NULL;
        int n = 0;
        ASSERT(
            index_get(idx, "inexistente", &occ, &n) != 0,
            "Palavra inexistente deveria falhar"
        );
    }

    /* ---------- TESTE: destruição ---------- */
    index_destroy(&idx);
    ASSERT(idx == NULL, "Indice nao foi destruido corretamente");

    printf("[OK] Todos os testes de indice passaram com sucesso.\n");
    return 0;
}
