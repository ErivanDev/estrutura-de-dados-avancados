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
typedef struct point point;

typedef enum { KEY, LEFT, RIGHT, COR } Tipo;
typedef enum { NEGRO, RUBRO } Cor;

Arn* CriarArvore();

no* CriarNo(point* segmento);

no* TrocarCor(no* x, int versao);

no* RotacaoESQ(no* x, int versao);

no* RotacaoDIR(no* x, int versao);

no* MoverEsquerda(no* x, int versao);

no* MoverDireita(no* x, int versao);

no* SalvarModificacao(no* z, Tipo tipo, int versao, void* valor);

no* IncluirRecursivo(no* x, float valor, point* segmento, int versao);

int Incluir(Arn* T, float valor, point* segmento);

no* Balancear(no* x, int versao);

no* RemoverMenor(no* x, int versao);

no* ProcurarMenor(no* x, int versao);

no* RemoverRecursivo(no* x, float valor, point* segmento, int versao);

int Consultar(Arn* T, float valor, point* segmento);

int Remover(Arn* T, float valor, point* segmento);

void PosOrdem(no* x, int nivel, int parent, char* dir);

void EmOrdem(no* x, int nivel, int parent, char* dir);

void PreOrdem(no* x, int nivel, float parent, char* d, int versao, float valor);