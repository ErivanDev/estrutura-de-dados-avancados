#include <stdio.h>
#include <stdlib.h>
#include "arn_3p.h"

typedef enum { NEGRO, RUBRO } Cor;

struct no {
    int        key;
    Cor        cor;
    struct no* pai;
    struct no* esq;
    struct no* dir;
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

no* CriarNo(int key) {
    no* z = (no*) malloc(sizeof(no));
    z->key = key;
    return z;
}

void RotacaoESQ(Arn *T, no* x) {
    no* y = x->dir;
    x->dir = y->esq;

    if ( y->esq != NIL_PTR ) {
        y->esq->pai = x;
    }

    y->pai = x->pai;
    
    if ( x->pai == NIL_PTR ) {
        T->raiz = y;
    }
    else if ( x == x->pai->esq ) {
        x->pai->esq = y;
    }
    else {
        x->pai->dir = y;
    }

    y->esq = x;
    x->pai = y;
}

void RotacaoDIR(Arn *T, no* x) {
    no* y = x->esq;
    x->esq = y->dir;

    if ( y->dir != NIL_PTR ) {
        y->dir->pai = x;
    }

    y->pai = x->pai;
    
    if ( x->pai == NIL_PTR ) {
        T->raiz = y;
    }
    else if ( x == x->pai->dir ) {
        x->pai->dir = y;
    }
    else {
        x->pai->esq = y;
    }

    y->dir = x;
    x->pai = y;
}

void ConsertarInclusao(Arn* T, no* z) {
    while ( z->pai->cor == RUBRO ) {
        // se o pai de 'z' é filho esquerdo
        if ( z->pai == z->pai->pai->esq ) {
            // pega o tio de 'z'
            no* y = z->pai->pai->dir;
            
            if ( y->cor == RUBRO ) {
                z->pai->cor = NEGRO;
                y->cor = NEGRO;
                z = y->pai;
                z->cor = RUBRO;
            }
            else {
                // no caso de 'z' ser filho direito
                if ( z == z->pai->dir ) {
                    z = z->pai;
                    RotacaoESQ(T, z);
                }

                z->pai->cor = NEGRO;
                z->pai->pai->cor = RUBRO;

                RotacaoDIR(T, z->pai->pai); // VERIFICAR
            }
        }
        else {
            // pega o tio de 'z'
            no* y = z->pai->pai->esq;
            
            if ( y->cor == RUBRO ) {
                z->pai->cor = NEGRO;
                y->cor = NEGRO;
                z = y->pai;
                z->cor = RUBRO;
            }
            else {
                // no caso de 'z' ser filho direito
                if ( z == z->pai->esq ) {
                    z = z->pai;
                    RotacaoDIR(T, z);
                }

                z->pai->cor = NEGRO;
                z->pai->pai->cor = RUBRO; 

                RotacaoESQ(T, z->pai->pai);
            }
        }
    }

    T->raiz->cor = NEGRO;
}

void Incluir(Arn* T, no* z) {
    no* y = NIL_PTR;
    no* x = T->raiz;

    while ( x != NIL_PTR) {
        y = x;

        if ( z->key < x->key ) {
            x = x->esq;
        }
        else {
            x = x->dir;
        }
    }

    z->pai = y;

    if ( y == NIL_PTR ) {
        T->raiz = z;
    }
    else if ( z->key < y->key ) {
        y->esq = z;
    }
    else {
        y->dir = z;
    }

    z->esq = z->dir = NIL_PTR;
    z->cor = RUBRO;
    
    ConsertarInclusao(T, z);
}

void Transplantar(Arn *T, no* u, no* v) {
    if ( u->pai == NIL_PTR ) {
        T->raiz = v;
    } 
    else if ( u == u->pai->esq ) {
        u->pai->esq = v;
    }
    else {
        u->pai->dir = v;
    }

    v->pai = u->pai;
}

no* Minimo(Arn *T, no* x) {
    while (x->esq != NIL_PTR) {
        x = x->esq;
    }
    return x;
}

no* Sucessor(Arn *T, no* x) {
    if (x->dir != NIL_PTR) {
        return Minimo(T, x->dir);
    }

    no* y = x->pai;
    while (y != NIL_PTR && x == y->dir) {
        x = y;
        y = y->pai;
    }
    return y;
}

void ConsertarRemocao(Arn *T, no* x) {
    while ( x != T->raiz && x->cor == NEGRO ) {
        if ( x == x->pai->esq ) {
            no* w = x->pai->dir;
            
            if ( w->cor == RUBRO ) {
                w->cor = NEGRO;
                x->pai->cor = RUBRO;
                RotacaoESQ(T, x->pai);
                w = x->pai->dir;
            }

            if ( w->esq->cor == NEGRO && w->dir->cor == NEGRO ) {
                w->cor = RUBRO;
                x = x->pai;
            }
            else {
                if ( w->dir->cor == NEGRO ) {
                    w->esq->cor = NEGRO;
                    w->cor = RUBRO;
                    RotacaoDIR(T, w);
                    w = x->pai->dir;
                }

                w->cor = x->pai->cor;
                x->pai->cor = NEGRO;
                w->dir->cor = NEGRO;
                RotacaoESQ(T, x->pai);
                x = T->raiz;
            }
        } 
        else {
            no* w = x->pai->esq;
            
            if ( w->cor == RUBRO ) {
                w->cor = NEGRO;
                x->pai->cor = RUBRO;
                RotacaoDIR(T, x->pai);
                w = x->pai->esq;
            }

            if ( w->dir->cor == NEGRO && w->esq->cor == NEGRO ) {
                w->cor = RUBRO;
                x = x->pai;
            }
            else {
                if ( w->esq->cor == NEGRO ) {
                    w->dir->cor = NEGRO;
                    w->cor = RUBRO;
                    RotacaoESQ(T, w);
                    w = x->pai->esq;
                }

                w->cor = x->pai->cor;
                x->pai->cor = NEGRO;
                w->esq->cor = NEGRO;
                RotacaoDIR(T, x->pai);
                x = T->raiz;
            }
        } 
    }

    x->cor = NEGRO;
}

void Remover(Arn *T, no* z) {
    no* y = z;
    int cor_original_y = y->cor;
    no* x;

    if ( z->esq == NIL_PTR ) {
        x = z->dir;
        Transplantar(T, z, x);
    }  
    else if ( z->dir == NIL_PTR ) {
        x = z->esq;
        Transplantar(T, z, x);
    }
    else {
        y = Sucessor(T, z);
        x = y->dir;
        cor_original_y = y->cor;
        Transplantar(T, y, x);
        y->esq = z->esq;
        z->esq->pai = y;
        y->dir = z->dir;
        z->dir->pai = y;
        Transplantar(T, z, y);
        y->cor = z->cor;
    }

    if ( cor_original_y == NEGRO ) {
        ConsertarRemocao(T, x);
    }
}

void Printar(no* z, char* dir, int nivel) {
    printf("valor %d, pai %d, direcao %s, nivel %d, cor %d.\n", z->key, z->pai->key, dir, nivel, z->cor);
    
    if (z->esq != NIL_PTR) {
        Printar(z->esq, "esq", nivel+1);
    }

    if (z->dir != NIL_PTR) { 
        Printar(z->dir, "dir", nivel+1);
    }
}

int main() {
    Arn* T = CriarArvore();

    int numeros[15] = {20, 30, 10, 15, 50, 60, 7, 3, 2, 25, 40, 55, 70, 22, 56};

    no* x;
    no* x1;
    for (int i = 0; i < 15; i++) {
        x = CriarNo(numeros[i]);

        if ( numeros[i] == 50 ) {
            x1 = x;
        }

        Incluir(T, x);
    }

    // Exibe o resultado
    no* z = T->raiz;
    Printar(z, "raiz", 0);

    printf("------------------------------\n");

    Remover(T, x1);
    // Exibe o resultado
    z = T->raiz;
    Printar(z, "raiz", 0);

    return 0;
}