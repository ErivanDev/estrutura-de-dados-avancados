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
typedef struct reta reta;
typedef struct ponto ponto;
typedef struct linha linha;

typedef enum { KEY, LEFT, RIGHT, COR } Tipo;
typedef enum { NEGRO, RUBRO } Cor;

// Função para calcular a coordenada y em uma reta dado um valor de x
float Calcular_y(float x, reta* r1);

// Função de comparação para ser usada na ordenação de um array
int Comparar(const void *a, const void *b);

// Função para remover duplicatas de um array e retornar o novo tamanho
int Remover_Duplicatas(int *array, int tamanho);

// Função para contar o número de blocos em um arquivo
int ContarBloco(FILE *arquivo);

// Função para ler as retas de um arquivo e armazená-las em um array
void LerRetas(FILE *arquivo, linha *retas, int quantidade);

// Função para ler os pontos de um arquivo e armazená-los em um array
void LerPontos (FILE *arquivo, ponto *pontos, int quantidade);

// Função para criar uma árvore rubro-negra
Arn* CriarArvore();

// Função para criar um novo nó
no* CriarNo(reta* segmento);

// Função para criar uma referência de acordo com o tipo
void* CriarReferencia(Tipo tipo, void* valor);

// Função para pegar uma modificacao de acordo com o tipo
void* PegarModificacao(no* z, Tipo tipo, int versao);

// Função para trocar a cor de um nó
no* TrocarCor(no* x, int versao);

// Função para realizar uma rotação à esquerda em um nó
no* RotacaoESQ(no* x, int versao);

// Função para realizar uma rotação à direita em um nó
no* RotacaoDIR(no* x, int versao);

// Função para mover o nó para a esquerda
no* MoverEsquerda(no* x, int versao);

// Função para mover o nó para a direita
no* MoverDireita(no* x, int versao);

// Função para salvar uma modificação
no* SalvarModificacao(no* z, Tipo tipo, int versao, void* valor);

// Função auxiliar para incluir um nó na arvore rubro-negra
no* IncluirRecursivo(no* x, float valor, reta* segmento, int versao);

// Função para incluir um nó em uma árvore rubro-negra
int Incluir(Arn* T, float valor, reta* segmento);

// Função para balancear uma árvore rubro-negra
no* Balancear(no* x, int versao);

// Função para remover o menor nó árvore rubro-negra
no* RemoverMenor(no* x, int versao);

// Função para procurar o menor nó em uma sub-arvore da arvore rubro-negra
no* ProcurarMenor(no* x, int versao);

// Função auxiliar para remover um nó da arvore rubro-negra
no* RemoverRecursivo(no* x, float valor, reta* segmento, int versao);

// Função para verificar se um nó está presente na arvore rubro-negra
int Consultar(Arn* T, float valor, reta* segmento);

// Função para remover um nó da arvore rubro-negra
int Remover(Arn* T, float valor, reta* segmento);

// Função para pegar o nó sucessor de uma sub-árvore de uma árvore rubro-negra
no* Sucessor(Arn* T, float valorX, float valorY, int versao);

// Função para imprimir em pós-ordem
void PosOrdem(no* x, int nivel, int parent, char* dir);

// Função para imprimir em ordem
void EmOrdem(no* x, int nivel, int parent, char* dir);

// Função para imprimir em pre-ordem
void PreOrdem(no* x, int nivel, float parent, char* d, int versao, float valor);