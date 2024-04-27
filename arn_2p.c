#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "arn_2p.h"

// Definindo constantes
#define MAX_OP 100 // <- número máximo de versões
#define MAX_QM 2 // <- número máximo de modificações por nó

///////////////////////////////////////////////////
/////// RESOLUCAO DO PROBLEMA DE LOCALIZACAO //////
///////////////////////////////////////////////////

// Definindo estruturas para representar pontos, linhas e retas 
struct ponto {
    float x;
    float y;
};

struct linha {
    ponto p1;
    ponto p2;
};

struct reta
{
    float x;
    float y;
    int r;
};

// Função para calcular a coordenada y em uma reta dado um valor de x
float Calcular_y(float x, reta* r1) {
    return r1->x * x + r1->y;
}

// Função de comparação para ser usada na ordenação de um array
int Comparar(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

// Função para remover duplicatas de um array e retornar o novo tamanho
int Remover_Duplicatas(int *array, int tamanho) {
    int i, j;
    if (tamanho == 0 || tamanho == 1)
        return tamanho;

    for (i = 0, j = 1; j < tamanho; j++) {
        if (array[j] != array[i]) {
            i++;
            array[i] = array[j];
        }
    }
    return i + 1;
}

// Função para contar o número de blocos em um arquivo
int ContarBloco(FILE *arquivo) {
    int valor;
    fscanf(arquivo, "%d", &valor);
    return valor;
}

// Função para ler as retas de um arquivo e armazená-las em um array
void LerRetas(FILE *arquivo, linha *retas, int quantidade) {
    for (int i = 0; i < quantidade; i++) {
        fscanf(arquivo, "%f,%f %f,%f", &retas[i].p1.x, &retas[i].p1.y, &retas[i].p2.x, &retas[i].p2.y);
    }
}

// Função para ler os pontos de um arquivo e armazená-los em um array
void LerPontos (FILE *arquivo, ponto *pontos, int quantidade) {
    for (int i = 0; i < quantidade; i++) {
        fscanf(arquivo, "%f,%f", &pontos[i].x, &pontos[i].y);
    }
}

///////////////////////////////////////////////////
/////////////////// PERSISTENCIA //////////////////
///////////////////////////////////////////////////

// Definindo estruturas para representar modificações em nós da árvore
struct SKey { reta* valor; };
struct SLeft { no* valor; };
struct SRight { no* valor; };
struct SCor { Cor valor; };

// Estrutura para representar uma modificação
typedef struct {
    int versao;
    Tipo tipo;
    void *ref;
} Mod;

///////////////////////////////////////////////////
///// ARVORE RUBRO-NEGRA CAIDA PARA A ESQUERDA ////
///////////////////////////////////////////////////

// Estrutura para representar um nó da árvore
struct no {
    struct reta*  key;
    struct no*     esq;
    struct no*     dir;
    Cor            cor;

    Mod*           mod[MAX_QM];
    int            qmods;
};

// Estrutura para representar uma árvore
struct Arn {
    struct no* raiz[MAX_OP];

    int ultima_versao;
};

// Nó NIL, utilizado para representar um nó nulo
no NIL_NO;
no* NIL_PTR = &NIL_NO;

// Função para criar uma árvore rubro-negra
Arn* CriarArvore() {
    Arn* T = (Arn*) malloc(sizeof(Arn));
    T->raiz[0] = NIL_PTR;
    return T;
}

// Função para criar um novo nó
no* CriarNo(reta* segmento) {
    no* z = (no*) malloc(sizeof(no));

    if ( segmento != NULL ) {
        z->key = segmento;
    }

    z->esq = z->dir = NIL_PTR;
    z->cor = RUBRO;
    return z;
}

// Função para criar uma referência de acordo com o tipo
void* CriarReferencia(Tipo tipo, void* valor) {
    void* ref = NULL;

    switch (tipo) {
        case KEY:   
            struct SKey* skey = (struct SKey*) malloc(sizeof(struct SKey));
            skey->valor = (reta*) valor;
            
            ref = (void* ) skey;
            
            break;
        case LEFT:  
            struct SLeft* sleft = (struct SLeft*) malloc(sizeof(struct SLeft));
            sleft->valor = (no*) valor;
            
            ref = (void*) sleft;
            
            break;
        case RIGHT: 
            struct SRight* sright = (struct SRight*) malloc(sizeof(struct SRight));
            sright->valor = (no*) valor;
            
            ref = (void*) sright;            
            break;
        case COR:   
            struct SCor* scor = (struct SCor*) malloc(sizeof(struct SCor));
            scor->valor = (Cor) valor;
            
            ref = (void*) scor;
            break;
        default: break;
    }

    return ref;
}

// Função para pegar uma modificacao de acordo com o tipo
void* PegarModificacao(no* z, Tipo tipo, int versao) {
    int i = 0;
    int modLen = sizeof(z->mod) / sizeof(z->mod[0]);
    void* ref;

    switch (tipo) {
        case KEY:   ref = CriarReferencia(tipo, z->key); break;
        case LEFT:  ref = CriarReferencia(tipo, z->esq); break;
        case RIGHT: ref = CriarReferencia(tipo, z->dir); break;
        case COR:   ref = CriarReferencia(tipo, (void*) z->cor); break;
        default: break;
    }

    while ( i < modLen && z->mod[i] != NULL && z->mod[i]->versao <= versao ) {
        if ( z->mod[i]->tipo == tipo ) {
            ref = z->mod[i]->ref;
        }

        i += 1;
    }

    return ref;
}

// Função para trocar a cor de um nó
no* TrocarCor(no* x, int versao) {
    Cor cor = ((struct SCor*) PegarModificacao(x, COR, versao))->valor;
    x = SalvarModificacao(x, COR, versao, (void*) !cor);
    
    no* x_esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;

    if (x_esq != NIL_PTR) {
        Cor x_esq_cor = ((struct SCor*) PegarModificacao(x_esq, COR, versao))->valor;
        no* new_x_esq = SalvarModificacao(x_esq, COR, versao, (void*) !x_esq_cor);

        if ( x_esq != new_x_esq ) {
            x = SalvarModificacao(x, LEFT, versao, new_x_esq);
        }
    }

    no* x_dir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
    
    if (x_dir != NIL_PTR) {
        Cor x_dir_cor = ((struct SCor*) PegarModificacao(x_dir, COR, versao))->valor;
        no* new_x_dir = SalvarModificacao(x_dir, COR, versao, (void*) !x_dir_cor);

        if ( x_dir != new_x_dir ) {
            x = SalvarModificacao(x, RIGHT, versao, new_x_dir);
        }
    }

    return x;
}

// Função para realizar uma rotação à esquerda em um nó
no* RotacaoESQ(no* x, int versao) {
    no* z = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
    no* z_esq = ((struct SLeft*) PegarModificacao(z, LEFT, versao))->valor;

    x = SalvarModificacao(x, RIGHT, versao, z_esq);
    z = SalvarModificacao(z, LEFT, versao, x);

    Cor x_cor = ((struct SCor*) PegarModificacao(x, COR, versao))->valor;

    z = SalvarModificacao(z, COR, versao, (void*) x_cor); // <- aqui eu posso criar um novo z

    no* oldX = x;
    x = SalvarModificacao(x, COR, versao, (void*) RUBRO); // <- aqui eu posso criar um novo x

    if ( oldX != x ) {
        z = SalvarModificacao(z, LEFT, versao, x);
    }

    return z;
}

// Função para realizar uma rotação à direita em um nó
no* RotacaoDIR(no* x, int versao) {
    no* z = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
    no* z_dir = ((struct SLeft*) PegarModificacao(z, RIGHT, versao))->valor;

    x = SalvarModificacao(x, LEFT, versao, z_dir);
    z = SalvarModificacao(z, RIGHT, versao, x);

    Cor x_cor = ((struct SCor*) PegarModificacao(x, COR, versao))->valor;

    z = SalvarModificacao(z, COR, versao, (void*) x_cor); // <- aqui eu posso criar um novo z

    no* oldX = x;
    x = SalvarModificacao(x, COR, versao, (void*) RUBRO); // <- aqui eu posso criar um novo x

    if ( oldX != x ) {
        z = SalvarModificacao(z, RIGHT, versao, x);
    }

    return z;
}

// Função para mover o nó para a esquerda
no* MoverEsquerda(no* x, int versao) {
    x = TrocarCor(x, versao);

    no* dir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
    Cor dirCor = ((struct SCor*) PegarModificacao(dir, COR, versao))->valor;
    no* dir_esq = ((struct SLeft*) PegarModificacao(dir, LEFT, versao))->valor;
    Cor dir_esqCor = ((struct SCor*) PegarModificacao(dir_esq, COR, versao))->valor;

    if ( dir_esqCor == RUBRO ) {
        no* oldDir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
        no* newDir = RotacaoDIR(oldDir, versao);

        if (oldDir != newDir) {
            x = SalvarModificacao(x, RIGHT, versao, newDir);
        }
        
        x = RotacaoESQ(x, versao);
        x = TrocarCor(x, versao);
    }

    return x;
}

// Função para mover o nó para a direita
no* MoverDireita(no* x, int versao) {
    x = TrocarCor(x, versao);

    no* esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
    Cor esqCor = ((struct SCor*) PegarModificacao(esq, COR, versao))->valor;
    no* esq_esq = ((struct SLeft*) PegarModificacao(esq, LEFT, versao))->valor;
    Cor esq_esqCor = ((struct SCor*) PegarModificacao(esq_esq, COR, versao))->valor;

    if ( esq_esqCor == RUBRO ) {
        x = RotacaoDIR(x, versao);
        x = TrocarCor(x, versao);
    }

    return x;
}

// Função para salvar uma modificação
no* SalvarModificacao(no* z, Tipo tipo, int versao, void* valor) {
    if ( z->qmods < MAX_QM ) {
        z->mod[z->qmods] = (Mod*) malloc(sizeof(Mod));

        z->mod[z->qmods]->versao = versao;
        z->mod[z->qmods]->tipo = tipo;

        z->mod[z->qmods]->ref = CriarReferencia(tipo, valor);

        z->qmods += 1;
    }
    else {
        // Cria copia
        no* y = CriarNo(NULL);

        reta* key = ((struct SKey*) PegarModificacao(z, KEY, versao))->valor;
        no* dir = ((struct SRight*) PegarModificacao(z, RIGHT, versao))->valor;
        no* esq = ((struct SLeft*) PegarModificacao(z, LEFT, versao))->valor;
        Cor cor = ((struct SCor*) PegarModificacao(z, COR, versao))->valor;

        y->key = key;
        y->dir = dir;
        y->esq = esq;
        y->cor = cor;

        z = y;

        // Adiciona modificacao
        z->mod[z->qmods] = (Mod*) malloc(sizeof(Mod));

        z->mod[z->qmods]->versao = versao;
        z->mod[z->qmods]->tipo = tipo;

        z->mod[z->qmods]->ref = CriarReferencia(tipo, valor);

        z->qmods += 1;
    }

    return z;
}

// Função auxiliar para incluir um nó na arvore rubro-negra
no* IncluirRecursivo(no* x, float valor, reta* segmento, int versao) {
    if (x == NIL_PTR) {
        no* z = CriarNo(segmento);
        
        return z;
    }
    
    reta* key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;
    
    float k1 = Calcular_y(valor, segmento);
    float k2 = Calcular_y(valor, key);

    if ( k1 == k2 ) {
        return x;
    }

    if ( k1 < k2 ) {
        no* oldEsq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
        no* newEsq = IncluirRecursivo(oldEsq, valor, segmento, versao);

        if (oldEsq != newEsq) {
            x = SalvarModificacao(x, LEFT, versao, newEsq);
        }
    }

    key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;

    k1 = Calcular_y(valor, segmento);
    k2 = Calcular_y(valor, key);

    if ( k1 > k2 ) {
        no* oldDir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
        no* newDir = IncluirRecursivo(oldDir, valor, segmento, versao);

        if (oldDir != newDir) {
            x = SalvarModificacao(x, RIGHT, versao, newDir);
        }
    }

    // nó vermelho sempre é o filho a esquerda, porque aqui se trata de uma left-leaning red–black tree
    no* dir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
    no* esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
    Cor dirCor = ((struct SCor*) PegarModificacao(dir, COR, versao))->valor;
    Cor esqCor = ((struct SCor*) PegarModificacao(esq, COR, versao))->valor;
    
    if ( dirCor == RUBRO && esqCor == NEGRO ) {
        x = RotacaoESQ(x, versao);
    }

    // se o filho da esquerda e o neto da esquerda são vermelhos 
    esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
    no* esq_esq = ((struct SLeft*) PegarModificacao(esq, LEFT, versao))->valor;
    esqCor = ((struct SCor*) PegarModificacao(esq, COR, versao))->valor;
    Cor esq_esqCor;

    if ( esq != NIL_PTR ) {
        esq_esqCor = ((struct SCor*) PegarModificacao(esq_esq, COR, versao))->valor;
    }

    if ( esq != NIL_PTR && esqCor == RUBRO && esq_esqCor == RUBRO ) {
        x = RotacaoDIR(x, versao);
    }

    // se 2 filhos vermelhos trocam-se as cores
    if ( x != NIL_PTR ) {
        dir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
        dirCor = ((struct SCor*) PegarModificacao(dir, COR, versao))->valor;
    }

    if ( x != NIL_PTR ) { 
        esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
        esqCor = ((struct SCor*) PegarModificacao(esq, COR, versao))->valor;
    }
    
    if ( x != NIL_PTR && esqCor == RUBRO && dirCor == RUBRO ) {
        x = TrocarCor(x, versao);
    }

    return x;
}

// Função para incluir um nó em uma árvore rubro-negra
int Incluir(Arn* T, float valor, reta* segmento) {
    if (!Consultar(T, valor, segmento)) {
        T->raiz[T->ultima_versao] = IncluirRecursivo(T->raiz[T->ultima_versao], valor, segmento, T->ultima_versao);

        if ( T->raiz[T->ultima_versao] != NIL_PTR ) {
            T->raiz[T->ultima_versao] = SalvarModificacao(T->raiz[T->ultima_versao], COR, T->ultima_versao, (void*) NEGRO);
        }

        T->raiz[T->ultima_versao + 1] = T->raiz[T->ultima_versao];

        T->ultima_versao += 1;
        return 1;
    }
    else {
        return 0;
    }
}

// Função para balancear uma árvore rubro-negra
no* Balancear(no* x, int versao) {
    // nó vermelho sempre é o filho a esquerda, porque aqui se trata de uma left-leaning red–black tree
    no* dir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
    Cor dirCor = ((struct SCor*) PegarModificacao(dir, COR, versao))->valor;

    if ( dirCor == RUBRO ) {
        x = RotacaoESQ(x, versao);
    }

    // se o filho da esquerda e o neto da esquerda são vermelhos 
    no* esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;    
    Cor esqCor = ((struct SCor*) PegarModificacao(esq, COR, versao))->valor;
    no* esq_esq = ((struct SLeft*) PegarModificacao(esq, LEFT, versao))->valor;
    Cor esq_esqCor;

    if ( esq != NIL_PTR ) {
        esq_esqCor = ((struct SCor*) PegarModificacao(esq_esq, COR, versao))->valor;
    }

    if ( esq != NIL_PTR && esqCor == RUBRO && esq_esqCor == RUBRO ) {
        x = RotacaoDIR(x, versao);
    }

    dir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
    dirCor = ((struct SCor*) PegarModificacao(dir, COR, versao))->valor;
    esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;    
    esqCor = ((struct SCor*) PegarModificacao(esq, COR, versao))->valor;

    // se 2 filhos vermelhos trocam-se as cores
    if ( esqCor == RUBRO && dirCor == RUBRO ) {
        x = TrocarCor(x, versao);
    }

    return x;
}

// Função para remover o menor nó árvore rubro-negra
no* RemoverMenor(no* x, int versao) {
    no* esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;

    if ( esq == NIL_PTR ) {
        return NIL_PTR;
    }

    esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;    
    Cor esqCor = ((struct SCor*) PegarModificacao(esq, COR, versao))->valor;
    no* esq_esq = ((struct SLeft*) PegarModificacao(esq, LEFT, versao))->valor;
    Cor esq_esqCor;

    if ( esq != NIL_PTR ) {
        esq_esqCor = ((struct SCor*) PegarModificacao(esq_esq, COR, versao))->valor;
    }

    if ( esqCor == NEGRO && esq_esqCor == NEGRO ) {
        x = MoverEsquerda(x, versao);
    }

    no* oldEsq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
    no* newEsq = RemoverMenor(oldEsq, versao);

    if (oldEsq != newEsq) {
        x = SalvarModificacao(x, LEFT, versao, newEsq);
    }
    
    return Balancear(x, versao);
}

// Função para procurar o menor nó em uma sub-arvore da arvore rubro-negra
no* ProcurarMenor(no* x, int versao) {
    no* z = x;
    no* y = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;

    while ( y != NIL_PTR ) {
        z = y;
        y = ((struct SLeft*) PegarModificacao(y, LEFT, versao))->valor;
    }
    
    return z;
}

// Função auxiliar para remover um nó da arvore rubro-negra
no* RemoverRecursivo(no* x, float valor, reta* segmento, int versao) {
    reta* key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;
    
    if ( Calcular_y(valor, segmento) < Calcular_y(valor, key) ) {
        no* esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
        no* esq_esq = ((struct SLeft*) PegarModificacao(esq, LEFT, versao))->valor;
        Cor esqCor = ((struct SCor*) PegarModificacao(esq, COR, versao))->valor;
        Cor esq_esqCor;

        if ( esq != NIL_PTR ) {
            esq_esqCor = ((struct SCor*) PegarModificacao(esq_esq, COR, versao))->valor;
        }

        if ( esqCor == NEGRO && esq_esqCor == NEGRO ) {
            x = MoverEsquerda(x, versao);
        }

        no* oldEsq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
        no* newEsq = RemoverRecursivo(oldEsq, valor, segmento, versao);

        if (oldEsq != newEsq) {
            x = SalvarModificacao(x, LEFT, versao, newEsq);
        }
    }
    else {
        no* esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
        Cor esqCor = ((struct SCor*) PegarModificacao(esq, COR, versao))->valor;

        if ( esqCor == RUBRO ) {
            x = RotacaoDIR(x, versao);
        }

        reta* key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;
        no* dir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
        
        if ( Calcular_y(valor, segmento) == Calcular_y(valor, key) && dir == NIL_PTR ) {
            return NIL_PTR;
        }

        dir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
        Cor dirCor = ((struct SCor*) PegarModificacao(dir, COR, versao))->valor;
        no* dir_esq = ((struct SLeft*) PegarModificacao(dir, LEFT, versao))->valor;
        Cor dir_esqCor = ((struct SCor*) PegarModificacao(dir_esq, COR, versao))->valor;

        if ( dirCor == NEGRO && dir_esqCor == NEGRO ) {
            x = MoverDireita(x, versao);
        }

        key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;
        dir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;

        if ( Calcular_y(valor, segmento) == Calcular_y(valor, key) ) {
            // copia o menor
            no* y = ProcurarMenor(dir, versao);

            reta* yKey = ((struct SKey*) PegarModificacao(y, KEY, versao))->valor;
            
            x = SalvarModificacao(x, KEY, versao, yKey);
            
            // remove o menor
            no* oldDir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
            no* newDir = RemoverMenor(oldDir, versao);

            if (oldDir != newDir) {
                x = SalvarModificacao(x, RIGHT, versao, newDir);
            }
        }
        else {
            no* oldDir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
            no* newDir = RemoverRecursivo(oldDir, valor, segmento, versao);

            if (oldDir != newDir) {
                x = SalvarModificacao(x, RIGHT, versao, newDir);
            }
        }
    }

    return Balancear(x, versao);
}

// Função para verificar se um nó está presente na arvore rubro-negra
int Consultar(Arn* T, float valor, reta* segmento) {
    if( T->raiz[T->ultima_versao] == NULL )
        return 0;

    no* x = T->raiz[T->ultima_versao];

    while( x != NIL_PTR ){
        reta* key = ((struct SKey*) PegarModificacao(x, KEY, T->ultima_versao))->valor;

        if ( Calcular_y(valor, segmento) == Calcular_y(valor, key) && segmento->r == key->r ) {
            return 1;
        }
        
        no* dir = ((struct SRight*) PegarModificacao(x, RIGHT, T->ultima_versao))->valor;
        reta* dirkey = ((struct SKey*) PegarModificacao(dir, KEY, T->ultima_versao))->valor;
        no* esq = ((struct SLeft*) PegarModificacao(x, LEFT, T->ultima_versao))->valor;

        if ( dirkey != NULL && Calcular_y(valor, segmento) >= Calcular_y(valor, dirkey) ) {
            x = dir;
        }
        else {
            x = esq;
        }
    }
    
    return 0;
}

// Função para remover um nó da arvore rubro-negra
int Remover(Arn* T, float valor, reta* segmento) {
    if (Consultar(T, valor, segmento)) {
        T->raiz[T->ultima_versao] = RemoverRecursivo(T->raiz[T->ultima_versao], valor, segmento, T->ultima_versao);

        if ( T->raiz[T->ultima_versao] != NIL_PTR ) {
            T->raiz[T->ultima_versao] = SalvarModificacao(T->raiz[T->ultima_versao], COR, T->ultima_versao, (void*) NEGRO);
        }

        T->raiz[T->ultima_versao + 1] = T->raiz[T->ultima_versao];

        T->ultima_versao += 1;

        return 1;
    }
    else {
        return 0;
    }
}

// Função para pegar o nó sucessor de uma sub-árvore de uma árvore rubro-negra
no* Sucessor(Arn* T, float valorX, float valorY, int versao) {
    struct no* x = T->raiz[versao];
    struct no* sucessor = NIL_PTR;

    while (x != NIL_PTR) {
        reta* key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;

        if ( Calcular_y(valorX, key) > valorY ) {
            sucessor = x;
            x = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
        } else {
            x = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
        }
    }

    return sucessor;
}

// Função para imprimir em pós-ordem
void PosOrdem(no* x, int nivel, int parent, char* dir) {
    if( x != NIL_PTR ){
        PosOrdem(x->esq, nivel+1, x->key, "<-");
        PosOrdem(x->dir, nivel+1, x->key, "->");

        if(x->cor == RUBRO)
            printf("%d  Vermelho: H(%d) P(%d) (%s) \n",x->key, nivel, parent, dir);
        else
            printf("%d  Preto:    H(%d) P(%d) (%s) \n",x->key, nivel, parent, dir);

    }
}

// Função para imprimir em ordem
void EmOrdem(no* x, int nivel, int parent, char* dir) {
    if( x != NIL_PTR ){
        EmOrdem(x->esq, nivel+1, x->key, "<-");

        if(x->cor == RUBRO)
            printf("%d  Vermelho: H(%d) P(%d) (%s) \n",x->key, nivel, parent, dir);
        else
            printf("%d  Preto:    H(%d) P(%d) (%s) \n",x->key, nivel, parent, dir);

        EmOrdem(x->dir, nivel+1, x->key, "->");
    }
}

// Função para imprimir em pre-ordem
void PreOrdem(no* x, int nivel, float parent, char* d, int versao, float valor) {
    if( x != NIL_PTR ) {
        no* dir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
        no* esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
        Cor cor = ((struct SCor*) PegarModificacao(x, COR, versao))->valor;
        reta* key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;

        if(cor == RUBRO)
            printf("%f  Vermelho: H(%d) P(%f) (%s) \n", Calcular_y(valor, key), nivel, parent, d);
        else
            printf("%f  Preto:    H(%d) P(%f) (%s) \n", Calcular_y(valor, key), nivel, parent, d);

        PreOrdem(esq, nivel+1, Calcular_y(valor, key), "<-", versao, valor);
        PreOrdem(dir, nivel+1, Calcular_y(valor, key), "->", versao, valor);
    }
}

// Função principal
int main(int argc, char *argv[]) {
    int enable_debug = 0;
    
    if (argc != 2 && argc != 3) {
        printf("Uso: %s <input_file>\n", argv[0]);
        return 1;
    }

    if (argc == 3) {
        char *char_debug = argv[2];
        enable_debug = atoi(char_debug);
        if ( enable_debug ) {
            printf("debug %d \n", enable_debug);
        }
    }

    char *input_file_path = argv[1];
    FILE *arquivo = fopen(input_file_path, "r");
    FILE *output = fopen("output.txt", "w");

    if (arquivo == NULL) {
        printf("Error opening input file.\n");
        return 1;
    }

    int numeroLinhas = ContarBloco(arquivo);

    printf("%d \n", numeroLinhas);
    fprintf(output, "%d \n", numeroLinhas);

    linha *retas = malloc(numeroLinhas * sizeof(linha));
    LerRetas(arquivo, retas, numeroLinhas);

    for (int i = 0; i < numeroLinhas; i++) {
        if (retas[i].p1.x == (int)retas[i].p1.x && retas[i].p1.y == (int)retas[i].p1.y && 
            retas[i].p2.x == (int)retas[i].p2.x && retas[i].p2.y == (int)retas[i].p2.y) {
            printf("%.0f,%.0f %.0f,%.0f\n", retas[i].p1.x, retas[i].p1.y, retas[i].p2.x, retas[i].p2.y);
            fprintf(output, "%.0f,%.0f %.0f,%.0f\n", retas[i].p1.x, retas[i].p1.y, retas[i].p2.x, retas[i].p2.y);
        } else {
            printf("%.1f,%.1f %.1f,%.1f\n", retas[i].p1.x, retas[i].p1.y, retas[i].p2.x, retas[i].p2.y);
            fprintf(output, "%.1f,%.1f %.1f,%.1f\n", retas[i].p1.x, retas[i].p1.y, retas[i].p2.x, retas[i].p2.y);
        }
    }

    int numeroPontos = ContarBloco(arquivo);

    printf("%d \n", numeroPontos);
    fprintf(output, "%d \n", numeroPontos);

    ponto *pontos = malloc(numeroPontos * sizeof(ponto));
    LerPontos(arquivo, pontos, numeroPontos);

    for (int i = 0; i < numeroPontos; i++) {
        if (pontos[i].x == (int)pontos[i].x && pontos[i].y == (int)pontos[i].y) {
            printf("%.0f,%.0f\n", pontos[i].x, pontos[i].y);
            fprintf(output, "%.0f,%.0f\n", pontos[i].x, pontos[i].y);
        } else {
            printf("%.1f,%.1f\n", pontos[i].x, pontos[i].y);
            fprintf(output, "%.1f,%.1f\n", pontos[i].x, pontos[i].y);
        }
    }

    Arn* T = CriarArvore();

    reta* segmento;

    int tx = numeroLinhas*2;
    float* xs = malloc(tx * sizeof(int));

    for (int i=0; i<numeroLinhas; i++) {
        xs[i] = retas[i].p1.x;
        xs[numeroLinhas+i] = retas[i].p2.x;
    }

    qsort(xs, tx, sizeof(int), Comparar);

    tx = Remover_Duplicatas(xs, tx);

    int nSegments = 0;
    int oldSegments = -1;
    float vSegmento = retas[0].p1.x;
    int intervalo = 0;
    
    int *intervalos = (int*) malloc((tx-1) * sizeof(int));
    int *rg = (int*) malloc(numeroLinhas * sizeof(int));

    for (int i = 0; i < numeroLinhas; i++) {
        rg[i] = 0;
    }

    if ( enable_debug ) {
        printf("\n");
        printf("novas retas \n");
    }

    for (int i=0; i<tx-1; i++) {
        if ( enable_debug ) {
            printf("intervalo %d \n", i);
        }

        for (int j=0; j<numeroLinhas; j++) {
            float x1 = retas[j].p1.x, y1 = retas[j].p1.y, x2 = retas[j].p2.x, y2 = retas[j].p2.y;

            float x = ( xs[i] + xs[i+1] )/2;
            float m = (y2 - y1) / (x2 - x1);
            float c = y1 - m * x1;
            float y = m * x + c;

            if ( x1 <= x && x2 >= x ) {
                rg[j] = 1;

                if ( vSegmento != x1 ) {
                    vSegmento = x1;
                }

                segmento = (reta*) malloc(sizeof(reta));
                segmento->x = m;
                segmento->y = c;
                segmento->r = j+1;

                int code = Incluir(T, x, segmento);

                if ( enable_debug ) {
                    printf("-------------- versão %d (%f) -------------------\n", nSegments, x);
                    printf("adicionar\n");
                    printf("+ m %f, c %f | %f %f %f %f \n ", segmento->x, segmento->y, x1, x2, y1, y2);
                    PreOrdem(T->raiz[nSegments], 0, (int) -INFINITY, "v", nSegments, x);
                }

                if (code) {
                    nSegments += 1;
                }
            }
            else {
                rg[j] = 0;

                segmento = (reta*) malloc(sizeof(reta));
                segmento->x = m;
                segmento->y = c;
                segmento->r = j+1;

                int foundedSameAngle = 0;

                for ( int k=0; k<numeroLinhas; k++ ) {
                    float j_x1 = retas[j].p1.x, j_y1 = retas[j].p1.y, j_x2 = retas[j].p2.x, j_y2 = retas[j].p2.y;

                    float j_x = ( xs[i] + xs[i+1] )/2;
                    float j_m = ( j_y2 - j_y1) / ( j_x2 - j_x1);
                    float j_c = j_y1 - m * j_x1;
                    float j_y = m * x + c;

                    float k_x1 = retas[k].p1.x, k_y1 = retas[k].p1.y, k_x2 = retas[k].p2.x, k_y2 = retas[k].p2.y;

                    float k_x = ( xs[i] + xs[i+1] )/2;
                    float k_m = ( k_y2 - k_y1) / ( k_x2 - k_x1);
                    float k_c = k_y1 - m * k_x1;
                    float k_y = k_m * k_x + k_c;

                    if ( j_y == k_y && j != k && ( k_x1 <= x && k_x2 >= x ) && k < j ) {
                        foundedSameAngle = 1;
                    }
                }
                
                if ( foundedSameAngle == 0 ) {
                    int code = Remover(T, x, segmento);

                    if ( enable_debug ) {
                        printf("-------------- versão %d (%f) -------------------\n", nSegments, x);
                        printf("remover\n");
                        printf("- m %f, c %f | %f %f %f %f \n ", segmento->x, segmento->y, x1, x2, y1, y2);
                        PreOrdem(T->raiz[nSegments], 0, (int) -INFINITY, "v", nSegments, x);
                    }

                    if ( code ) {
                        nSegments += 1;
                    }
                }
                else {
                    if ( enable_debug ) {
                        printf("-------------- versão %d (%f) -------------------\n", nSegments, x);
                        printf("remover\n");
                        printf("- m %f, c %f | %f %f %f %f \n ", segmento->x, segmento->y, x1, x2, y1, y2);
                        PreOrdem(T->raiz[nSegments], 0, (int) -INFINITY, "v", nSegments, x);
                    }
                }
            }
        }

        if ( enable_debug ) {
            printf("\n\nSegmentos presentes: ");
            for (int i = 0; i < numeroLinhas; ++i) {
                printf("%d ", rg[i]);
            }
            printf("\n\n");
        }

        intervalos[i] = T->ultima_versao-1;
    }

    if ( enable_debug ) {
        for (int i=0; i<tx-1; i++) {
            printf("%d, ", intervalos[i]);
        }
        printf("\n");

        for (int i=0; i<tx-1; i++) {
            float x = (xs[i] + xs[i+1])/2;
            printf("-------------------  intervalo %d -----------------------\n", i + 1);

            PreOrdem(T->raiz[intervalos[i]], 0, (int) -INFINITY, "v", intervalos[i], x);
        }
    }

    printf("\n");
    fprintf(output, "\n");

    for (int i=0; i<numeroPontos; i++) {
        int intervaloAtual = 0;
        float xAtual = xs[0];
        float x1 = pontos[i].x;
        float y1 = pontos[i].y;

        int idx = 0; 
        while (x1 >= xs[idx+1]) {
            idx += 1;
        }

        no* sucessor = Sucessor(T, x1, y1, intervalos[idx]);
        
        if ( sucessor != NIL_PTR ) {
            printf("%d \n", sucessor->key->r);
            fprintf(output, "%d \n", sucessor->key->r);
        }
        else {
            printf("%d \n", -1);
            fprintf(output, "%d \n", -1);
        }
    }

    return 0;
}