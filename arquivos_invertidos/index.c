#include "index.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LINEBUF 4096
#define HASH_PRIMO 1009
#define KEY_MAX 16

/**
 * @brief Internal structure representing a keyword entry.
 *
 * Each entry stores:
 *  - the normalized keyword
 *  - a dynamic array of line numbers where the keyword occurs
 *  - a pointer to the next entry in the same hash bucket (chaining)
 */
typedef struct Entrada {
    char key[KEY_MAX + 1];
    int *linhas;
    int count_linhas, cap;
    struct Entrada *prox;
} Entrada;

/**
 * @brief Opaque index structure (hash table).
 *
 * This structure is intentionally hidden from users of the API.
 * It represents a hash table with chaining for collision resolution.
 */
struct index {
    int num_buckets;
    Entrada **buckets;
};

/* ============================================================
   Internal helper functions (not exposed in index.h)
   ============================================================ */

/**
 * @brief Removes trailing newline characters from a string.
 *
 * Modifies the string in place by removing '\n' and '\r'
 * characters at the end.
 *
 * @param s Input string.
 */
static void trim_newline(char *s) {
    if (!s) return;
    int n = (int)strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }
}

/**
 * @brief Normalizes an ASCII string.
 *
 * This function:
 *  - removes leading and trailing whitespace
 *  - converts all characters to lowercase
 *
 * @param s Input string to be normalized.
 */
static void normalize_ascii(char *s) {
    if (!s) return;

    /* trim left */
    int i = 0;
    while (s[i] && isspace((unsigned char)s[i])) i++;
    if (i) memmove(s, s + i, strlen(s + i) + 1);

    /* trim right */
    int n = (int)strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) {
        s[n - 1] = '\0';
        n--;
    }

    /* lowercase */
    for (int k = 0; s[k]; k++) {
        s[k] = (char)tolower((unsigned char)s[k]);
    }
}

/**
 * @brief DJB2 hash function for strings.
 *
 * @param s Null-terminated string.
 * @return Hash value.
 */
static unsigned long hash_djb2(const char *s) {
    unsigned long h = 5381u;
    unsigned char c;
    while ((c = (unsigned char)*s++)) {
        h = (h * 33u) + c;
    }
    return h;
}

/**
 * @brief Frees a single index entry.
 *
 * @param e Pointer to the entry.
 */
static void entrada_free(Entrada *e) {
    if (!e) return;
    free(e->linhas);
    free(e);
}

/**
 * @brief Creates an empty index with a given number of buckets.
 *
 * @param num_buckets Number of hash buckets.
 * @return Pointer to the newly created index, or NULL on failure.
 */
static Index *index_create_empty(int num_buckets) {
    if (num_buckets <= 0) return NULL;

    Index *idx = (Index *)malloc(sizeof(Index));
    if (!idx) return NULL;

    idx->num_buckets = num_buckets;
    idx->buckets = (Entrada **)malloc((size_t)num_buckets * sizeof(Entrada *));
    if (!idx->buckets) {
        free(idx);
        return NULL;
    }

    for (int i = 0; i < num_buckets; i++) idx->buckets[i] = NULL;
    return idx;
}

/**
 * @brief Destroys an index and frees all associated memory.
 *
 * @param idx Pointer to the index pointer.
 * @return 0 on success, non-zero on error.
 */
int index_destroy(Index **idx) {
    if (!idx || !*idx) return 1;

    Index *p = *idx;
    for (int b = 0; b < p->num_buckets; b++) {
        Entrada *cur = p->buckets[b];
        while (cur) {
            Entrada *nx = cur->prox;
            entrada_free(cur);
            cur = nx;
        }
        p->buckets[b] = NULL;
    }

    free(p->buckets);
    free(p);
    *idx = NULL;
    return 0;
}

/**
 * @brief Searches for a normalized keyword in the index.
 *
 * @param idx Index.
 * @param key_norm Normalized keyword.
 * @param bucket_out Optional pointer to store the bucket index.
 * @return Pointer to the entry if found, NULL otherwise.
 */
static Entrada *busca_index(Index *idx, const char *key_norm, int *bucket_out) {
    if (!idx || !key_norm || idx->num_buckets <= 0) return NULL;

    unsigned long h = hash_djb2(key_norm);
    int b = (int)(h % (unsigned long)idx->num_buckets);

    if (bucket_out) *bucket_out = b;

    for (Entrada *e = idx->buckets[b]; e; e = e->prox) {
        if (strcmp(e->key, key_norm) == 0) return e;
    }
    return NULL;
}

/**
 * @brief Inserts a normalized keyword into the index.
 *
 * If the keyword already exists, nothing is done.
 *
 * @param idx Index.
 * @param key_norm Normalized keyword.
 * @return 0 on success, non-zero on allocation error.
 */
static int insere_index_norm(Index *idx, const char *key_norm) {
    if (!idx || !key_norm) return 1;

    int bucket = 0;
    if (busca_index(idx, key_norm, &bucket)) return 0;

    Entrada *e = (Entrada *)malloc(sizeof(Entrada));
    if (!e) return 2;

    strncpy(e->key, key_norm, KEY_MAX);
    e->key[KEY_MAX] = '\0';
    e->linhas = NULL;
    e->count_linhas = 0;
    e->cap = 0;

    e->prox = idx->buckets[bucket];
    idx->buckets[bucket] = e;
    return 0;
}

/**
 * @brief Adds a line number to an entry occurrence list.
 *
 * @param e Entry.
 * @param line_no Line number.
 * @return 0 on success, non-zero on allocation error.
 */
static int add_line(Entrada *e, int line_no) {
    if (!e) return 1;

    if (e->count_linhas == e->cap) {
        int novo_cap = (e->cap == 0) ? 4 : e->cap * 2;
        int *tmp = (int *)realloc(e->linhas, (size_t)novo_cap * sizeof(int));
        if (!tmp) return 2;
        e->linhas = tmp;
        e->cap = novo_cap;
    }

    e->linhas[e->count_linhas++] = line_no;
    return 0;
}

/**
 * @brief Tokenizes a line and records keyword occurrences.
 *
 * Tokens consist of letters, digits or underscores.
 *
 * @param idx Index.
 * @param line Input text line.
 * @param line_no Line number in the text file.
 */
static void analisa_tokens(Index *idx, const char *line, int line_no) {
    if (!idx || !line) return;

    char tok[KEY_MAX + 1];
    int t = 0;

    for (int i = 0;; i++) {
        unsigned char uc = (unsigned char)line[i];
        char c = (char)uc;

        int is_word = (c != 0) && (isalnum(uc) || c == '_');

        if (is_word) {
            if (t < KEY_MAX) tok[t++] = (char)tolower(uc);
        }

        if (!is_word) {
            if (t > 0) {
                tok[t] = '\0';
                Entrada *e = busca_index(idx, tok, NULL);
                if (e) (void)add_line(e, line_no);
                t = 0;
            }
            if (c == 0) break;
        }
    }
}

/**
 * @brief Comparator for alphabetical sorting of entries.
 */
static int cmp_entrada(const void *a, const void *b) {
    const Entrada *ea = *(const Entrada *const *)a;
    const Entrada *eb = *(const Entrada *const *)b;
    return strcmp(ea->key, eb->key);
}

/* ============================================================
   Public API functions (declared in index.h)
   ============================================================ */

/**
 * @brief Inserts a keyword into the index.
 *
 * @param idx Index.
 * @param key Keyword (not normalized).
 * @return 0 on success, non-zero on error.
 */
int index_put(Index *idx, const char *key) {
    if (!idx || !key) return 1;

    char key_norm[KEY_MAX + 1];
    strncpy(key_norm, key, KEY_MAX);
    key_norm[KEY_MAX] = '\0';
    normalize_ascii(key_norm);

    if (key_norm[0] == '\0') return 2;

    return insere_index_norm(idx, key_norm);
}

/**
 * @brief Creates an index from a keyword file and a text file.
 *
 * @param key_file File containing keywords.
 * @param text_file File to be indexed.
 * @param idx Output index.
 * @return 0 on success, non-zero on error.
 */
int index_createfrom(const char *key_file, const char *text_file, Index **idx) {
    if (!idx || !key_file || !text_file) return 1;
    *idx = NULL;

    Index *out = index_create_empty(HASH_PRIMO);
    if (!out) return 2;

    FILE *fk = fopen(key_file, "r");
    if (!fk) {
        index_destroy(&out);
        return 3;
    }

    char buf[LINEBUF];

    while (fgets(buf, sizeof(buf), fk)) {
        trim_newline(buf);
        if (buf[0] == '\0') continue;

        if (index_put(out, buf) != 0) {
            fclose(fk);
            index_destroy(&out);
            return 4;
        }
    }
    fclose(fk);

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
 * @brief Retrieves all occurrences of a keyword.
 *
 * @param idx Index.
 * @param key Keyword.
 * @param occurrences Output array of line numbers.
 * @param num_occurrences Number of occurrences.
 * @return 0 on success, non-zero on error.
 */
int index_get(const Index *idx, const char *key, int **occurrences, int *num_occurrences) {
    if (!idx || !key || !occurrences || !num_occurrences) return 1;

    *occurrences = NULL;
    *num_occurrences = 0;

    char key_norm[KEY_MAX + 1];
    strncpy(key_norm, key, KEY_MAX);
    key_norm[KEY_MAX] = '\0';
    normalize_ascii(key_norm);

    Entrada *e = busca_index((Index *)idx, key_norm, NULL);
    if (!e) return 2;

    if (e->count_linhas <= 0) return 0;

    int *v = (int *)malloc((size_t)e->count_linhas * sizeof(int));
    if (!v) return 3;

    for (int i = 0; i < e->count_linhas; i++) v[i] = e->linhas[i];

    *occurrences = v;
    *num_occurrences = e->count_linhas;
    return 0;
}

/**
 * @brief Prints the entire index in alphabetical order.
 *
 * @param idx Index.
 * @return 0 on success, non-zero on error.
 */
int index_print(const Index *idx) {
    if (!idx) return 1;

    int total = 0;
    for (int b = 0; b < idx->num_buckets; b++)
        for (Entrada *e = idx->buckets[b]; e; e = e->prox)
            total++;

    if (total == 0) return 0;

    Entrada **vet = (Entrada **)malloc((size_t)total * sizeof(Entrada *));
    if (!vet) return 2;

    int k = 0;
    for (int b = 0; b < idx->num_buckets; b++)
        for (Entrada *e = idx->buckets[b]; e; e = e->prox)
            vet[k++] = e;

    qsort(vet, (size_t)total, sizeof(Entrada *), cmp_entrada);

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

