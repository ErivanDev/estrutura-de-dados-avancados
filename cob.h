#include <stdio.h>
#include <stdlib.h>

typedef struct vetor_auxiliar vetor_auxiliar;
typedef struct limite limite;
typedef struct intervalo intervalo;
typedef struct arvore_layout_veb arvore_layout_veb;
typedef struct COB COB;

static inline int bsf_word(int word);
static inline int bsr_word(int word) ;

limite CalcularLimitedeDensidade(vetor_auxiliar* vetor, int profundidade);
int AcharFolha(vetor_auxiliar* vetor, int indice);
int AcharNo(int indice, int tamanho);
void DeslocarDireita(vetor_auxiliar* vetor, int indice);
double CalcularDensidade(vetor_auxiliar* vetor, int indice, int tamanho);
void Redistribuir(vetor_auxiliar* vetor, int indice, int tamanho);
void TableDoubling(vetor_auxiliar* vetor, arvore_layout_veb* arvoreLVEB);
void TableHalving(vetor_auxiliar* vetor, arvore_layout_veb* arvoreLVEB);
void PrintarVetor(vetor_auxiliar* vetor, FILE* saida);
void IniciarVetor(vetor_auxiliar* vetor);
intervalo Inserir(vetor_auxiliar* vetor, arvore_layout_veb* arvoreLVEB, int indice, int valor);
intervalo Deletar(vetor_auxiliar* vetor, arvore_layout_veb* arvoreLVEB, int indice);

extern const int anotacao_altura_arvore_baixo[];

int ProximaPotenciade2(int n);
void GerarOrdemVanEmdeBoas(arvore_layout_veb* arvoreLVEB, int arvore_indice, int altura, int altura_original, int bfs_ordem);
void CriarArvoreLayoutVanEmdeBoas(arvore_layout_veb* arvoreLVEB, int indice, int altura);
// void Atualizar(arvore_layout_veb* arvoreLVEB, int no_atual, int altura, int indice, int valor);
void Atualizar(arvore_layout_veb* arvoreLVEB, vetor_auxiliar* vetor,int no_atual, int alturaO, int altura, int profundidade, int indice, int inicial, int final);
void AtualizarArvore(arvore_layout_veb* arvoreLVEB, int indice, int valor);
void CriarArvoreLVEB(arvore_layout_veb* arvoreLVEB, int n);
int ProcurarNo(arvore_layout_veb* arvoreLVEB, int valor);

COB* CriarCOB();
void PrintarDebugCOB(COB* cob, FILE* saida);
void IniciarVetor(vetor_auxiliar* vetor);