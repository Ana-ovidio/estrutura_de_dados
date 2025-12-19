
## Ideia geral

O programa trabalha com **dois arquivos de entrada**:

* `keys.txt` → contém as **palavras-chave de interesse**
* `texto.txt` → contém o **texto a ser analisado**

O funcionamento é dividido em duas etapas:

1. As palavras de `keys.txt` são inseridas no índice (sem ocorrências ainda)
2. O arquivo `texto.txt` é varrido linha a linha, e cada ocorrência de uma palavra-chave é registrada com o número da linha

Depois disso, o usuário pode **consultar uma palavra** e ver **em quais linhas ela aparece**.

---

## Organização do projeto

```
.
├── include/
│   ├── tad.h
│   ├── index_utils.h
│   ├── tooling.h
│   └── etl.h
│
├── src/
│   ├── index_utils.c
│   ├── tooling.c
│   ├── etl.c
│   └── main.c
│
├── data/
│   ├── keys.txt
│   └── texto.txt
│
├── build/
├── output/
└── Makefile
```

### Principais módulos

* **`index_utils`**
  Implementa a estrutura do índice (hash, busca, criação e destruição).

* **`tooling`**
  Responsável pelo tratamento de texto (normalização, tokenização e registro de ocorrências).

* **`etl`**
  Coordena a construção do índice a partir dos arquivos (`keys.txt` e `texto.txt`) e fornece funções de consulta.

* **`main`**
  Interface com o usuário (leitura de argumentos, consulta e exibição dos resultados).

---

## Como compilar

No diretório do projeto, execute:

```bash
make
```

Isso irá compilar todos os arquivos e gerar o executável em `output/index`.

---

## ▶ Como executar

O programa deve ser executado informando **dois arquivos**:

```bash
make run
```

ou manualmente:

```bash
./output/main data/keys.txt data/texto.txt
```

---

## Exemplo de uso

### Arquivo `keys.txt`

```
index
data
tree
```

### Arquivo `texto.txt`

```
This index is simple
Data structures are important
The index helps organize data
```

### Execução

```
Qual a palavra-chave a procurar?
index
```

### Saída

```
2 ocorrencias de index: 1, 3
```

---

## 🧩 Observações importantes

* Apenas palavras presentes em `keys.txt` são indexadas
* A busca **não diferencia maiúsculas de minúsculas**
* A memória é totalmente liberada ao final da execução
* O projeto segue o padrão de **TAD (Tipo Abstrato de Dados)**, separando interface e implementação


