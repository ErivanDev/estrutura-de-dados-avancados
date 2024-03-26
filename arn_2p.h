/*
Propriedades da Arvore Rubro Negra
1   Todo nó é RUBRO ou NEGRO
2   A raiz é NEGRO
3   Todo nó externo é NEGRO
4   Se um nó é RUBRO, então seu filhos são NEGROS
5   Para todo caminho entre a raiz e um nó externo tem o mesmo
    número de nós NEGRO
*/

typedef struct Arn Arn;
typedef struct no no;

Arn* CriarArvore();

no* CriarNo(int valor);

void TrocarCor(no* x);

no* RotacaoESQ(no* x);

no* RotacaoDIR(no* x);

no* MoverEsquerda(no* x);

no* MoverDireita(no* x);

no* IncluirRecursivo(no* x, int valor);

void Incluir(Arn* T, int valor);

no* Balancear(no* x);

no* RemoverMenor(no* x);

no* ProcurarMenor(no* x);

no* RemoverRecursivo(no* x, int valor);

int Consultar(Arn* T, int valor);

void Remover(Arn* T, int valor);

void PosOrdem(no* x, int nivel, int parent, char* dir);

void EmOrdem(no* x, int nivel, int parent, char* dir);

void PreOrdem(no* x, int nivel, int parent, char* dir);