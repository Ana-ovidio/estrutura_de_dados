#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINEBUF 4096
#define HASH_PRIMO 1009

#define KEY_MAX 16


/*
- key: Vetor de caracteres que armazena a palavra-chave
- linhas: Ponteiro para inteiro que aponto para o endereço das linhas onde key apareceu
- count_linhas: Quantas linhas já foram guardadas
- cap: Capacidade atual do vetor
- prox: aponta pra endereço do próximo TAD
*/
typedef struct Entrada {
    char key[KEY_MAX + 1];
    int *linhas;
    int count_linhas, cap;
    struct Entrada *prox;
} Entrada;

/*
- num_buckets: Número de buckets
- buckets: Em cada bucket, fica uma lista encadeada de palavras
Obs: Cada palavra é uma Entrada
*/
struct Index {
    int num_buckets;
    Entrada **buckets;
};
typedef struct Index Index;   



Index *index_create_empty(int num_buckets);
int index_destroy(Index **idx);

void trim_newline(char *s);
void normalize_ascii(char *s);

Entrada *busca_index(Index *idx, const char *key_norm, int *bucket_out);
int insere_index(Index *idx, const char *key_norm);
void analisa_tokens(Index *idx, char *line, int line_no);

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
