#ifndef INDEX_H
#define INDEX_H

#ifdef __cplusplus
extern "C" {
#endif

#define KEY_MAX 16

typedef struct Index Index;

int index_createfrom(const char *key_file, const char *text_file, Index **idx);
int index_destroy(Index **idx);
int index_get(const Index *idx, const char *key, int **occurrences, int *num_occurrences);
int index_print(const Index *idx);

#ifdef __cplusplus
}
#endif

#endif
