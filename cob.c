#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cob.h"

#define max(a, b) ((a) > (b) ? (a) : (b))

/*
    Vetor Auxiliar
*/

typedef struct vetor_auxiliar {
	int N;
	int H;
	int logN;
	int* itens;
} vetor_auxiliar;

typedef struct limite {
	double limite_inferior;
	double limite_superior;
} limite;

typedef struct intervalo {
	int inicio;
	int final;
} intervalo;

static inline int bsf_word(int word) {
    return __builtin_ctz(word);
}

static inline int bsr_word(int word) {
    return 31 - __builtin_clz(word);
}

limite CalcularLimitedeDensidade(vetor_auxiliar* vetor, int profundidade) {
	limite lim;

	lim.limite_inferior = 1.0 / 4.0 + (vetor->H != 0 ? (.125 * profundidade) / vetor->H : 0);
	lim.limite_superior = 3.0 / 4.0 + (vetor->H != 0 ? (.25 * profundidade) / vetor->H : 0);

	return lim;
}

int AcharFolha(vetor_auxiliar* vetor, int indice) {
	return (indice/vetor->logN)*vetor->logN;
}

int AcharNo(int indice, int tamanho) {
	return (indice/tamanho)*tamanho;
}

void DeslocarDireita(vetor_auxiliar* vetor, int indice) {
	int elemento = vetor->itens[indice];

	while (vetor->itens[++indice] != -1) {
		int temp = vetor->itens[indice];
		vetor->itens[indice] = elemento;
		elemento = temp;
	}

	vetor->itens[indice] = elemento;
}

double CalcularDensidade(vetor_auxiliar* vetor, int indice, int tamanho) {
	int quantidade = 0;

	for (int i = indice; i < indice+tamanho; i++) {
		quantidade += (vetor->itens[i] != -1);
	}
	
    double quantidade_double = (double) quantidade;
	return quantidade_double/tamanho;
}

void Redistribuir(vetor_auxiliar* vetor, int indice, int tamanho) {
	int *temp = (int*) malloc(tamanho*sizeof(*(vetor->itens)));
	int j = 0;

	// colocar todos os valores dentro de um vetor temporario
    for (int i = indice; i < indice+tamanho; i++) {
		temp[j] = vetor->itens[i];
		j+=(vetor->itens[i]!=-1);
		vetor->itens[i] = -1;
	}

    // redistribuir uniformemente para uma densidade uniforme
	double indice_double = indice;
	double passos = ((double) tamanho)/j;
	for (int i = 0; i < j; i++) {
	  int in = indice_double;
	  vetor->itens[in] = temp[i];
	  indice_double+=passos;
	}
    
	free(temp);
}

void TableDoubling(vetor_auxiliar* vetor, arvore_layout_veb* arvoreLVEB) {
	vetor->N*=2;
	vetor->logN = (1 << bsr_word(bsr_word(vetor->N)+1));
	vetor->H = bsr_word(vetor->N/vetor->logN);
	vetor->itens = (int*) realloc(vetor->itens, vetor->N*sizeof(*(vetor->itens)));
	for (int i = vetor->N/2; i < vetor->N; i++) {
		vetor->itens[i] = -1;
	}
	Redistribuir(vetor, 0, vetor->N);

	CriarArvoreLVEB(arvoreLVEB, vetor->N);
}

void TableHalving(vetor_auxiliar* vetor, arvore_layout_veb* arvoreLVEB) {
	vetor->N/=2;
	vetor->logN = (1 << bsr_word(bsr_word(vetor->N)+1));
	vetor->H = bsr_word(vetor->N/vetor->logN);
	
    int *novo_vetor = (int*) malloc(vetor->N*sizeof(*(vetor->itens)));
	int j = 0;
	
    for (int i = 0; i < vetor->N*2; i++) {
		if (vetor->itens[i] != -1) {
			novo_vetor[j++] = vetor->itens[i];
		}
    }
	
    for (int i = j; j < vetor->N; j++) {
		novo_vetor[j] = -1;
	}
	
    free(vetor->itens);
	vetor->itens = novo_vetor;

	Redistribuir(vetor, 0, vetor->N);

	CriarArvoreLVEB(arvoreLVEB, vetor->N);
}

void PrintarVetor(vetor_auxiliar* vetor, FILE* saida) {
	for (int i = 0; i < vetor->N; i++) {
		if (vetor->itens[i] == -1) {
			// printf("/ ");
		} else {
			printf("%d ", vetor->itens[i]);
			fprintf(saida, "%d ", vetor->itens[i]);
		}
	}
	printf("\n");
}

void IniciarVetor(vetor_auxiliar* vetor) {
	vetor->N = 1;
	vetor->logN = (1 << bsr_word(bsr_word(vetor->N)+1));
	vetor->H = bsr_word(vetor->N/vetor->logN);
	vetor->itens = (int*)malloc(vetor->N*sizeof(*(vetor->itens)));

	for (int i = 0; i < vetor->N; i++) {
		vetor->itens[i] = -1;
	}
}

intervalo Inserir(vetor_auxiliar* vetor, arvore_layout_veb* arvoreLVEB, int indice, int valor) {
	// usando manutencao de arquivos ordenados, o vetor é dividido em log n valores dos espacos
	// essa função retorna o indice inicial desse folha de log n indices
	int indice_no = AcharFolha(vetor, indice);
	int nivel = vetor->H;
	int tamanho = vetor->logN;

	// se tiver espaço coloca, se não cria espaço dislocando para a direita
	if (vetor->itens[indice] == -1) {
		vetor->itens[indice] = valor;
	} else {
		DeslocarDireita(vetor, indice);
		vetor->itens[indice] = valor;
	}

	double densidade = CalcularDensidade(vetor, indice_no, tamanho);

	// o espaço da folha está cheia
	if (densidade == 1) {
		// pega a proxima folha ou a folha anterior para distribuir
	  	indice_no = AcharNo(indice_no, tamanho*2);

	  	Redistribuir(vetor, indice_no, tamanho*2);
	} else {
		// redistribui só na folha
		Redistribuir(vetor, indice_no, tamanho);
	}

	limite lim = CalcularLimitedeDensidade(vetor, nivel);
	densidade = CalcularDensidade(vetor, indice_no, tamanho);

	// verifica as densidades subindo os niveis, procurando qual densidade não está correta
	while (densidade >= lim.limite_superior) {
		tamanho*=2;
		if (tamanho <= vetor->N){
			nivel--;
			indice_no = AcharNo(indice_no, tamanho);
			lim = CalcularLimitedeDensidade(vetor, nivel);
			densidade = CalcularDensidade(vetor, indice_no, tamanho);
		} else {
			TableDoubling(vetor, arvoreLVEB);
			
			intervalo intervaloAlterado;
			intervaloAlterado.inicio = indice_no;
			intervaloAlterado.final = indice_no + tamanho;
			
			return intervaloAlterado;
		}
	}

	// redistribua a partir do nó que não respeita a densidade
	Redistribuir(vetor, indice_no, tamanho);

	intervalo intervaloAlterado;
	intervaloAlterado.inicio = indice_no;
	intervaloAlterado.final = indice_no + tamanho;

	return intervaloAlterado;
}

intervalo Deletar(vetor_auxiliar* vetor, arvore_layout_veb* arvoreLVEB, int indice) {
    // usando manutencao de arquivos ordenados, o vetor é dividido em log n valores dos espacos
	// essa função retorna o indice inicial desse folha de log n indices
	int indice_no = AcharFolha(vetor, indice);
	int nivel = vetor->H;
	int tamanho = vetor->logN;

	limite lim = CalcularLimitedeDensidade(vetor, nivel);

	if(vetor->itens[indice]== -1){
		// printf("Este elemento nao existe neste indice: %d \n", indice);

		intervalo intervaloAlterado;
		intervaloAlterado.inicio = -1;
		intervaloAlterado.final = -1;

		return intervaloAlterado;
	}
	
	// deleção
	vetor->itens[indice] = -1;

	// redistribuir 'recursivamente' até estar dentro dos limites de densidade
	double densidade = CalcularDensidade(vetor, indice_no, tamanho);

	while(densidade < lim.limite_inferior) {
		tamanho*=2;
		if (tamanho <= vetor->N){
			nivel--;
			indice_no = AcharNo(indice_no, tamanho);
			lim = CalcularLimitedeDensidade(vetor, nivel);
			densidade = CalcularDensidade(vetor, indice_no, tamanho);
		} else {
			TableHalving(vetor, arvoreLVEB);

			intervalo intervaloAlterado;
			intervaloAlterado.inicio = indice_no;
			intervaloAlterado.final = indice_no + tamanho;

			return intervaloAlterado;
		}
	}

	Redistribuir(vetor, indice_no, tamanho);

	intervalo intervaloAlterado;
	intervaloAlterado.inicio = indice_no;
	intervaloAlterado.final = indice_no + tamanho;

	return intervaloAlterado;
}

/* 
    Cache Oblivious BTree
*/

typedef struct arvore_layout_veb {
	int N;
    int comprimento;
    int tamanho;
    int altura;

    int* ordem;
    int* arvore;
} arvore_layout_veb;

int ProximaPotenciade2(int n) {
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n; 
}

int CalcularAlturaArvoreBaixo(int altura ) {
	return ceil(pow(2, ceil(log2(altura)) + 1) / 4);
}

// ALTURA IGUAL A 3
//    0
//  1   4
// 2 3 5 6

// ALTURA IGUAL A 4
// 
//         0
//    1         2
//  3   6    9    12
// 4 5 7 8 10 11 13 14

// ALTURA IGUAL A 5
//
//                    0
//          1                     16
//    2          3          17         18
//  4   7    10    13   19     22    25   28
// 5 6 8 9 11 12 14 15 20 21 23 24 26 27 29 30

void GerarOrdemVanEmdeBoas(arvore_layout_veb* arvoreLVEB, int arvore_indice, int altura, int altura_original, int bfs_ordem) {
    if (altura == 1)
    {
        arvoreLVEB->ordem[bfs_ordem] = arvore_indice;
        return;
    }

    int altura_arvore_baixo = CalcularAlturaArvoreBaixo(altura);
    int altura_arvore_cima = altura - altura_arvore_baixo;

    GerarOrdemVanEmdeBoas(arvoreLVEB, arvore_indice, altura_arvore_cima, altura_original, bfs_ordem);
    
    int tamanho_arvore_cima = (1 << altura_arvore_cima) - 1;

    int altura_original_arvore_baixo = altura_original - altura_arvore_cima;
    int tamanho_arvore_baixo = (1 << altura_arvore_baixo) - 1;
    int subarvore_indice = arvore_indice + tamanho_arvore_cima;

    for (int arvore_baixo_indices = 0; arvore_baixo_indices < (1 << altura_arvore_cima); ++arvore_baixo_indices) {
        int bottom_tree_bfs_order = (bfs_ordem << altura_arvore_cima) | arvore_baixo_indices;
        GerarOrdemVanEmdeBoas(arvoreLVEB, subarvore_indice, altura_arvore_baixo, altura_original_arvore_baixo, bottom_tree_bfs_order);
        subarvore_indice += tamanho_arvore_baixo;
    }
}

// n1v n1e  n1d  n1ev n1ee n1ed  n1eev n1eee n1eed n1edv n1ede n1edd n1dv n1de n1dd n1dev n1dee n1ded ...
//   0   3   12     0    6    9      0     0     0     0     0     0    0   15   18    0      0     0 ...

void CriarArvoreLayoutVanEmdeBoas(arvore_layout_veb* arvoreLVEB, int indice, int altura) {
    if(altura == 1) {
        return;
    }
    int indice_van_emde_boas = arvoreLVEB->ordem[indice];
    arvoreLVEB->arvore[indice_van_emde_boas * 3 + 1] = arvoreLVEB->ordem[indice << 1] * 3;
    arvoreLVEB->arvore[indice_van_emde_boas * 3 + 2] = arvoreLVEB->ordem[(indice << 1) + 1] * 3;
    
    CriarArvoreLayoutVanEmdeBoas(arvoreLVEB, indice << 1, altura-1);
    CriarArvoreLayoutVanEmdeBoas(arvoreLVEB, (indice << 1) + 1, altura-1);
}

//    13
//  5    13
// 1 5  9 13

// 4 0100
// 2 0010
// 1 0001

// se o indice for 5, 0101
// 0100 & 0101 = 0100, maior que 4
// 0010 & 0101 = 0000, menor que 6
// 0001 & 0101 = 0001, maior que 4, logo é 5

//                                  0                                                - 1
//                    00                               01                            - 2
//      000               001                010                011                  - 3
// 0000    0001      0010     0011      0100     0101      0110      0111            - 4
//   0       1         2        3         4        5         6         7 
/*void Atualizar(arvore_layout_veb* arvoreLVEB, int no_atual, int altura, int indice, int valor) {
    if(altura == 1) {
        arvoreLVEB->arvore[no_atual] = valor;
        return;
    }
	if(indice & (1 << (altura - 2))) {
        Atualizar(arvoreLVEB, arvoreLVEB->arvore[no_atual + 2], altura - 1, indice, valor);
    } else {
        Atualizar(arvoreLVEB, arvoreLVEB->arvore[no_atual + 1], altura - 1, indice, valor);
    }
    arvoreLVEB->arvore[no_atual] = max(arvoreLVEB->arvore[arvoreLVEB->arvore[no_atual + 1]], arvoreLVEB->arvore[arvoreLVEB->arvore[no_atual + 2]]);
}*/

void Atualizar(arvore_layout_veb* arvoreLVEB, vetor_auxiliar* vetor,int no_atual, int alturaO, int altura, int profundidade, int indice, int inicial, int final) {
    if(altura == 1) {
        arvoreLVEB->arvore[no_atual] = vetor->itens[indice];
		// printf("indice %d \n", indice);
        return;
    }

	int inicialFE = (indice << 1) << (alturaO - profundidade - 2);
	int inicialFD = ((indice << 1) + 1) << (alturaO - profundidade - 2);
	int metadeDoTamanho = (inicialFD - inicialFE) / 2;

	// printf("altura %d profundidade %d inicial range [%d] \n", alturaO, profundidade, indice << (alturaO - profundidade));
	// printf("meioFE: %d \n", inicialFE + metadeDoTamanho);
	// printf("meioFD: %d \n", inicialFD + metadeDoTamanho);
    
	if ( inicial < inicialFE + metadeDoTamanho )
    	Atualizar(arvoreLVEB, vetor, arvoreLVEB->arvore[no_atual + 1], alturaO, altura - 1, profundidade + 1, indice << 1, inicial, final);
	
	if ( final > inicialFD + metadeDoTamanho )
		Atualizar(arvoreLVEB, vetor, arvoreLVEB->arvore[no_atual + 2], alturaO, altura - 1, profundidade + 1, (indice << 1) + 1, inicial, final);

    arvoreLVEB->arvore[no_atual] = max(arvoreLVEB->arvore[arvoreLVEB->arvore[no_atual + 1]], arvoreLVEB->arvore[arvoreLVEB->arvore[no_atual + 2]]);
}

void AtualizarArvore(arvore_layout_veb* arvoreLVEB, int indice, int valor) {
    // Atualizar(arvoreLVEB, 0, arvoreLVEB->altura, indice, valor);
}

void CriarArvoreLVEB(arvore_layout_veb* arvoreLVEB, int n) {
	arvoreLVEB->N = n;
    arvoreLVEB->comprimento = ProximaPotenciade2(arvoreLVEB->N);
    arvoreLVEB->tamanho = arvoreLVEB->comprimento * 2;
    arvoreLVEB->altura = log2(arvoreLVEB->comprimento) + 1;
    
    arvoreLVEB->ordem = (int*) malloc(sizeof(int) * arvoreLVEB->tamanho);
    arvoreLVEB->arvore = (int*) malloc(sizeof(int) * arvoreLVEB->tamanho * 3);

	GerarOrdemVanEmdeBoas(arvoreLVEB, 0, arvoreLVEB->altura, arvoreLVEB->altura, 1);
    CriarArvoreLayoutVanEmdeBoas(arvoreLVEB, 1, arvoreLVEB->altura);
}

int ProcurarNo(arvore_layout_veb* arvoreLVEB, int valor) {
    int no_atual = 0;
    int indice = 0;
    for (int i = 0; i < arvoreLVEB->altura - 1; ++i)
    {
        if (valor <= arvoreLVEB->arvore[arvoreLVEB->arvore[no_atual + 1]])
        {
            no_atual = arvoreLVEB->arvore[no_atual + 1];
            indice <<= 1;
        }
        else
        {
            no_atual = arvoreLVEB->arvore[no_atual + 2];
            indice <<= 1;
            indice += 1;
        }
    }
    return indice;
}

int Sucessor(arvore_layout_veb* arvoreLVEB, int valor) {
    int no_atual = 0;
    int indice = 0;
    for (int i = 0; i < arvoreLVEB->altura - 1; ++i)
    {
        if (valor < arvoreLVEB->arvore[arvoreLVEB->arvore[no_atual + 1]])
        {
            no_atual = arvoreLVEB->arvore[no_atual + 1];
            indice <<= 1;
        }
        else
        {
            no_atual = arvoreLVEB->arvore[no_atual + 2];
            indice <<= 1;
            indice += 1;
        }
    }
    return indice;
}

typedef struct COB {
	arvore_layout_veb* arvore;
    vetor_auxiliar* vetor;
} COB;

COB* CriarCOB() {
    COB* cob = (COB*) malloc(sizeof(COB));

    cob->arvore = (arvore_layout_veb*) malloc(sizeof(arvore_layout_veb));
    cob->vetor = (vetor_auxiliar*) malloc(sizeof(vetor_auxiliar));

	CriarArvoreLVEB(cob->arvore, 1);
    IniciarVetor(cob->vetor);

    return cob;
}

void PrintarDebugCOB(COB* cob, FILE* saida) {
    // printf("------------------------------------------------\n");

    int n = cob->vetor->N;

    /*printf("Ordem van emde boas: %d\n", n*2*3);
    for (int i = 0; i < n*2; ++i){
        printf("%d ", cob->ordem[i]);
    }
    printf("\n");
    printf("\n");*/

    /*printf("Arvore max: %d\n", n*2*3);
    for (int i = 0; i < n*2*3; ++i){
        printf("%d ", cob->arvore->arvore[i]);
    }
    printf("\n");
    printf("\n");*/

    // printf("Vetor auxiliar: %d\n", n);
    PrintarVetor(cob->vetor, saida);

    // printf("------------------------------------------------\n");
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
        printf("Uso: %s <input_file>\n", argv[0]);
        return 1;
    }

	COB* cob = CriarCOB();

	char *input_file_path = argv[1];
    FILE *arquivo = fopen(input_file_path, "r");
	FILE *saida = fopen("output.txt", "w");
    
	if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return 0;
    }

    char line[100];
    char command[10];
    int valor_aleatorio;

	while (fgets(line, sizeof(line), arquivo)) {
        printf(line);
		fprintf(saida, line);
	}
	
	printf("\n\n");
	fprintf(saida, "\n\n");

	rewind(arquivo);

    while (fgets(line, sizeof(line), arquivo)) {
        if (sscanf(line, "%s %d", command, &valor_aleatorio) == 2) {
        	if (strcmp(command, "INC") == 0) {
				int indice = ProcurarNo(cob->arvore, valor_aleatorio);
        		intervalo interA = Inserir(cob->vetor, cob->arvore, indice, valor_aleatorio);
				Atualizar(cob->arvore, cob->vetor, 0, cob->arvore->altura, cob->arvore->altura, 0, 0, interA.inicio-1, interA.final+1);
			}

			if (strcmp(command, "REM") == 0) {
				int indice = ProcurarNo(cob->arvore, valor_aleatorio);
				intervalo interA = Deletar(cob->vetor, cob->arvore, indice);

				Atualizar(cob->arvore, cob->vetor, 0, cob->arvore->altura, cob->arvore->altura, 0, 0, interA.inicio-1, interA.final+1);
			}

			if (strcmp(command, "SUC") == 0) {
				int indice = Sucessor(cob->arvore, valor_aleatorio);

				printf("%d \n", cob->vetor->itens[indice]);
				fprintf(saida, "%d \n", cob->vetor->itens[indice]);
			}
        } else {
            PrintarDebugCOB(cob, saida);
        }
    }

    fclose(arquivo);
	fclose(saida);
	
    return 0;
}

/*int main(int argc, char *argv[]) {
    COB* cob = CriarCOB();

    PrintarDebugCOB(cob);

	for (int i=0; i<8; i++) {
        int valor_aleatorio = rand() % 101;

        int indice = ProcurarNo(cob->arvore, valor_aleatorio);
        intervalo interA = Inserir(cob->vetor, cob->arvore, indice, valor_aleatorio);

		// printf("lim inf %d, lim sup %d \n", interA.inicio, interA.final);

        // for (int i = 0; i < cob->vetor->N; ++i) {
        // 	AtualizarArvore(cob->arvore, i, cob->vetor->itens[i]);
        // }

		Atualizar(cob->arvore, cob->vetor, 0, cob->arvore->altura, cob->arvore->altura, 0, 0, interA.inicio-1, interA.final+1);
		
        PrintarDebugCOB(cob);
    }

	vetor_auxiliar* aux = malloc(sizeof(vetor_auxiliar));
	if (aux != NULL) {
		aux->N = cob->vetor->N;
		aux->H = cob->vetor->H;
		aux->logN = cob->vetor->logN;
		
		aux->itens = malloc(aux->N * sizeof(int));
		if (aux->itens != NULL) {
			memcpy(aux->itens, cob->vetor->itens, aux->N * sizeof(int));
		}
	}

	int tamanho = aux->N;
    int* array = malloc(tamanho * sizeof(int));

    for (int i = 0; i < tamanho; i++) array[i] = i;

	for (int i = tamanho - 1; i > 0; i--) {
        int j = rand() % (i + 1); 
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }

	for (int i = 0; i < tamanho; i++) {
		if ( aux->itens[array[i]] != -1 ) {
			int indice = ProcurarNo(cob->arvore, aux->itens[array[i]]);
			intervalo interA = Deletar(cob->vetor, cob->arvore, indice);

			Atualizar(cob->arvore, cob->vetor, 0, cob->arvore->altura, cob->arvore->altura, 0, 0, interA.inicio-1, interA.final+1);

			PrintarDebugCOB(cob);
		}
	}

    return 0;
}*/