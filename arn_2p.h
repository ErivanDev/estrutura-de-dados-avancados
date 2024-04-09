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

typedef enum { KEY, LEFT, RIGHT, COR } Tipo;
typedef enum { NEGRO, RUBRO } Cor;

Arn* CriarArvore();

no* CriarNo(int valor);

no* TrocarCor(no* x, int versao);

no* RotacaoESQ(no* x, int versao);

no* RotacaoDIR(no* x, int versao);

no* MoverEsquerda(no* x, int versao);

no* MoverDireita(no* x, int versao);

no* SalvarModificacao(no* z, Tipo tipo, int versao, void* valor);

no* IncluirRecursivo(no* x, int valor, int versao);

void Incluir(Arn* T, int valor);

no* Balancear(no* x, int versao);

no* RemoverMenor(no* x, int versao);

no* ProcurarMenor(no* x, int versao);

no* RemoverRecursivo(no* x, int valor, int versao);

int Consultar(Arn* T, int valor);

void Remover(Arn* T, int valor);

void PosOrdem(no* x, int nivel, int parent, char* dir);

void EmOrdem(no* x, int nivel, int parent, char* dir);

void PreOrdem(no* x, int nivel, int parent, char* dir, int versao);