#ifndef TAD_H
#define TAD_H

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

#endif