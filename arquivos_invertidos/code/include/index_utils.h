#ifndef INDEX_UTILS_H
#define INDEX_UTILS_H

#include "tad.h"

Index *index_create_empty(int num_buckets);
int index_destroy(Index **idx);

Entrada *busca_index(Index *idx, const char *key_norm, int *bucket_out);

#endif
