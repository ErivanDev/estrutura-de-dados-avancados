#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "arn_2p.h"

typedef enum { NEGRO, RUBRO } Cor;

struct no {
    int        key;
    struct no* esq;
    struct no* dir;
    Cor        cor;
};

struct Arn {
    struct no* raiz;
};

no NIL_NO;
no* NIL_PTR = &NIL_NO;

Arn* CriarArvore() {
    Arn* T = (Arn*) malloc(sizeof(Arn));
    T->raiz = NIL_PTR;
    return T;
}

no* CriarNo(int valor) {
    no* z = (no*) malloc(sizeof(no));
    z->key = valor;
    z->esq = z->dir = NIL_PTR;
    z->cor = RUBRO;
    return z;
}

void TrocarCor(no* x) {
    x->cor = !x->cor;
    
    if (x->esq != NIL_PTR) {
        x->esq->cor = !x->esq->cor;
    }
    
    if (x->dir != NIL_PTR) {
        x->dir->cor = !x->dir->cor;
    }
}

no* RotacaoESQ(no* x) {
    no* z = x->dir;

    x->dir = z->esq;
    z->esq = x;
    z->cor = x->cor;
    x->cor = RUBRO;

    return z;
}

no* RotacaoDIR(no* x) {
    no* z = x->esq;

    x->esq = z->dir;
    z->dir = x;
    z->cor = x->cor;
    x->cor = RUBRO;

    return z;
}

no* MoverEsquerda(no* x) {
    TrocarCor(x);

    if ( x->dir->esq->cor == RUBRO ) {
        x->dir = RotacaoDIR(x->dir);
        x = RotacaoESQ(x);
        TrocarCor(x);
    }

    return x;
}

no* MoverDireita(no* x) {
    TrocarCor(x);

    if ( x->esq->esq->cor == RUBRO ) {
        x = RotacaoDIR(x);
        TrocarCor(x);
    }

    return x;
}

no* IncluirRecursivo(no* x, int valor) {
    if (x == NIL_PTR) {
        no* z = CriarNo(valor);
        
        return z;
    }
    
    if (valor < x->key) {
        x->esq = IncluirRecursivo(x->esq, valor);
    }
    
    if (valor > x->key) {
        x->dir = IncluirRecursivo(x->dir, valor);
    }

    // nó vermelho sempre é o filho a esquerda, porque aqui se trata de uma left-leaning red–black tree
    if ( x->dir->cor == RUBRO && x->esq->cor == NEGRO ) {
        x = RotacaoESQ(x);
    }

    // se o filho da esquerda e o neto da esquerda são vermelhos 
    if ( x->esq->cor == RUBRO && x->esq->esq != NIL_PTR && x->esq->esq->cor == RUBRO ) {
        x = RotacaoDIR(x);
    }

    // se 2 filhos vermelhos trocam-se as cores
    if ( x->esq->cor == RUBRO && x->dir->cor == RUBRO ) {
        TrocarCor(x);
    }

    return x;
}

void Incluir(Arn* T, int valor) {
    T->raiz = IncluirRecursivo(T->raiz, valor);

    if ( T->raiz != NIL_PTR ) {
        T->raiz->cor = NEGRO;
    }
}

no* Balancear(no* x) {
    // nó vermelho sempre é o filho a esquerda, porque aqui se trata de uma left-leaning red–black tree
    if ( x->dir->cor == RUBRO ) {
        x = RotacaoESQ(x);
    }

    // se o filho da esquerda e o neto da esquerda são vermelhos 
    if ( x->esq != NIL_PTR && x->esq->cor == RUBRO && x->esq->esq->cor == RUBRO ) {
        x = RotacaoDIR(x);
    }

    // se 2 filhos vermelhos trocam-se as cores
    if ( x->esq->cor == RUBRO && x->dir->cor == RUBRO ) {
        TrocarCor(x);
    }

    return x;
}

no* RemoverMenor(no* x) {
    if ( x->esq == NIL_PTR ) {
        free(x);
        return NIL_PTR;
    }
    
    if ( x->esq->cor == NEGRO && x->esq->esq == NEGRO ) {
        x = MoverEsquerda(x);
    }

    x->esq = RemoverMenor(x->esq);
    
    return Balancear(x);
}

no* ProcurarMenor(no* x) {
    no* z = x;
    no* y = x->esq;

    while ( y != NIL_PTR ) {
        z = y;
        y = y->esq;
    }
    
    return z;
}

no* RemoverRecursivo(no* x, int valor) {
    if ( valor < x->key ) {
        if ( x->esq->cor == NEGRO && x->esq->esq->cor == NEGRO ) {
            x = MoverEsquerda(x);
        }

        x->esq = RemoverRecursivo(x->esq, valor);
    }
    else {
        if ( x->esq->cor == RUBRO ) {
            x = RotacaoDIR(x);
        }

        if ( x->key == valor && x->dir == NIL_PTR ) {
            free(x);
            return NIL_PTR;
        }

        if ( x->dir->cor == NEGRO && x->dir->esq->cor == NEGRO ) {
            x = MoverDireita(x);
        }

        if ( x->key == valor ) {
            // copia o menor
            no* y = ProcurarMenor(x->dir);
            x->key = y->key;
            // remove o menor
            x->dir = RemoverMenor(x->dir);
        }
        else {
            x->dir = RemoverRecursivo(x->dir, valor);
        }
    }

    return Balancear(x);
}


int Consultar(Arn* T, int valor) {
    if( T->raiz == NULL )
        return 0;

    no* x = T->raiz;

    while( x != NIL_PTR ){
        if( valor == x->key ) {
            return 1;
        }

        if( valor >= x->dir->key ) {
            x = x->dir;
        }
        else {
            x = x->esq;
        }
    }
    
    return 0;
}

void Remover(Arn* T, int valor) {
    if (Consultar(T, valor)) {
        T->raiz = RemoverRecursivo(T->raiz, valor);

        if ( T->raiz != NIL_PTR ) {
            T->raiz->cor = NEGRO;
        }
    }
}

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

void PreOrdem(no* x, int nivel, int parent, char* dir) {
    if( x != NIL_PTR ){
        if(x->cor == RUBRO)
            printf("%d  Vermelho: H(%d) P(%d) (%s) \n",x->key, nivel, parent, dir);
        else
            printf("%d  Preto:    H(%d) P(%d) (%s) \n",x->key, nivel, parent, dir);

        PreOrdem(x->esq, nivel+1, x->key, "<-");
        PreOrdem(x->dir, nivel+1, x->key, "->");
    }
}

int main() {
    Arn* T = CriarArvore();

    int numeros[15] = {20, 30, 10, 15, 50, 60, 7, 3, 2, 25, 40, 55, 70, 22, 56};

    for (int i = 0; i < 9; i++) {
        Incluir(T, numeros[i]);
        printf("------------------------------\n");
        PreOrdem(T->raiz, 0, (int) -INFINITY, "v");
    }

    /*for (int i = 0; i < 15; i++) {
        Incluir(T, numeros[i]);
    }*/

    // Exibe o resultado
    // PreOrdem(T->raiz, 0, (int) -INFINITY, "v");

    // printf("------------------------------\n");]

    printf("------------------------------\n");
    Remover(T, 50);
    PreOrdem(T->raiz, 0, (int) -INFINITY, "v");
    printf("------------------------------\n");
    Remover(T, 20);
    PreOrdem(T->raiz, 0, (int) -INFINITY, "v");
    printf("------------------------------\n");
    Remover(T, 3);
    PreOrdem(T->raiz, 0, (int) -INFINITY, "v");
    printf("------------------------------\n");
    Remover(T, 15);
    PreOrdem(T->raiz, 0, (int) -INFINITY, "v");
    printf("------------------------------\n");
    Remover(T, 30);
    PreOrdem(T->raiz, 0, (int) -INFINITY, "v");
    printf("------------------------------\n");
    Remover(T, 60);
    PreOrdem(T->raiz, 0, (int) -INFINITY, "v");
    printf("------------------------------\n");


    // Exibe o resultado
    // PreOrdem(T->raiz, 0, (int) -INFINITY, "v");

    // printf("------------------------------\n");

    // Exibe o resultado
    // PreOrdem(T->raiz, 0, (int) -INFINITY, "v");

    return 0;
}