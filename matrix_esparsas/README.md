# Matrizes Esparsas em C

Este projeto implementa operações básicas com **matrizes esparsas** em C, usando listas encadeadas para armazenar apenas os elementos não nulos.

O programa permite criar matrizes via entrada padrão e realizar operações como soma, transposição e multiplicação.

---

## Como compilar

Use o `gcc`:

```bash
gcc *.c -o matriz
```

Se estiver usando um `Makefile`, basta:

```bash
make
```

---

## Como executar

Após compilar:

```bash
make run
```

O programa vai pedir:

1. As dimensões da matriz (linhas e colunas)
2. Os elementos no formato `i j valor`
3. Digite `0` para encerrar a entrada de dados

---

## Principais funções

### Criação e destruição

* `init_matrix(linhas, colunas)`
  Cria uma matriz esparsa vazia.

* `matrix_create(Matrix **m)`
  Cria uma matriz lendo os dados da entrada padrão.

* `matrix_destroy(Matrix *m)`
  Libera toda a memória associada à matriz.

### Acesso e modificação

* `matrix_setelem(m, i, j, valor)`
  Insere ou atualiza o elemento `(i, j)`.
  Se `valor == 0`, o elemento é removido.

* `matrix_getelem(m, i, j, &valor)`
  Obtém o valor do elemento `(i, j)`.

### Operações matemáticas

* `matrix_add(m, n, &r)`
  Soma duas matrizes de mesmas dimensões.

* `matrix_transpose(m, &r)`
  Calcula a transposta da matriz.

* `matrix_multiply(m, n, &r)`
  Multiplica duas matrizes compatíveis.

---

## Estrutura dos arquivos

* `create.c`
  Criação, inserção e destruição das matrizes.

* `manipulate.c`
  Funções de acesso e modificação de elementos.

* `math.c`
  Operações matemáticas (soma, transposta e multiplicação).

* `inputs.c`
  Funções auxiliares para leitura de dados do usuário.

* `*.h`
  Definições das estruturas (`Matrix`, `No`) e protótipos das funções.

---

## Observações

* A indexação das matrizes começa em **1** (linha e coluna).
* Apenas elementos não nulos são armazenados.
* As listas encadeadas de cada linha são mantidas ordenadas por coluna.


