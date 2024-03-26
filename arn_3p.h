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

no* CriarNo(int key);

void RotacaoESQ(Arn *T, no* x);

void RotacaoDIR(Arn *T, no* x);

void ConsertarInclusao(Arn *T, no* z);

void Incluir(Arn *T, no* z);

void Transplantar(Arn *T, no* u, no* v);

no* Minimo(Arn *T, no* x);

no* Sucessor(Arn *T, no* x);

void ConsertarRemocao(Arn *T, no* x);

void Remover(Arn *T, no* z);

void Printar(no* z, char* dir, int nivel);