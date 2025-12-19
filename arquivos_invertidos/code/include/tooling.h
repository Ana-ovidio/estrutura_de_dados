#ifndef TOOLING_H
#define TOOLING_H

#include "tad.h"
#include "index_utils.h"

void trim_newline(char *s);
void normalize_ascii(char *s);
int insere_index(Index *idx, const char *key_norm);
void analisa_tokens(Index *idx, char *line, int line_no);

#endif
