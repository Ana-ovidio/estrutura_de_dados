#ifndef INDEX_H
#define INDEX_H

#include "tad.h"


int index_createfrom(
    const char *key_file,
    const char *text_file,
    Index **idx
);

int index_get(
    const Index *idx,
    const char *key,
    int **occurrences,
    int *num_occurrences
);

int index_print(const Index *idx);

#endif
