# Árvore B Independente de Cache

1. Como compilar:

```
gcc cob.c -o cob -lm
```

2. Como executar:

```
Uso: ./cob <input_file>

Exemplo: 
     ./cob exemplo_io_1.txt
```

3. Passos para a resolução do problema:

- 1º Passo: Implementação de uma Árvore Binaria no layout Van Emde Boas para a busca.

    ```
    typedef struct arvore_layout_veb {
        int N;
        int comprimento;
        int tamanho;
        int altura;

        int* ordem;
        int* arvore;
    } arvore_layout_veb;
    ```

- 2º Passo: Implementação de um vetor auxiliar. 

    ```
    typedef struct vetor_auxiliar {
        int N;
        int H;
        int logN;
        int* itens;
    } vetor_auxiliar;
    ```