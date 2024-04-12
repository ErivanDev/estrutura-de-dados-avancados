#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "arn_2p.h"

#define MAX_OP 20
#define MAX_QM 2

struct point
{
    float x;
    float y;
};

/////// PERSISTENCIA //////

struct SKey { point* valor; };
struct SLeft { no* valor; };
struct SRight { no* valor; };
struct SCor { Cor valor; };

typedef struct {
    int versao;
    Tipo tipo;
    void *ref;
} Mod;

///////////////////////////

float fixedFloat(float valor) {
    return valor;
    // float rounded_result = (int)(valor * 100000.0) / 100000.0;
    // return rounded_result;
}

float calcular_y(float x, point* r1) {
    return r1->x * x + r1->y;
    // return fixedFloat( r1->x * x + r1->y );
}

struct no {
    struct point*  key;
    struct no*     esq;
    struct no*     dir;
    Cor            cor;

    Mod*           mod[MAX_QM];
    int            qmods;
};

struct Arn {
    struct no* raiz[MAX_OP];

    int ultima_versao;
};

no NIL_NO;
no* NIL_PTR = &NIL_NO;

Arn* CriarArvore() {
    Arn* T = (Arn*) malloc(sizeof(Arn));
    T->raiz[0] = NIL_PTR;
    return T;
}

no* CriarNo(point* segmento) {
    no* z = (no*) malloc(sizeof(no));

    if ( segmento != NULL ) {
        z->key = segmento;
    }

    z->esq = z->dir = NIL_PTR;
    z->cor = RUBRO;
    return z;
}

void* CriarReferencia(Tipo tipo, void* valor) {
    void* ref = NULL;

    switch (tipo) {
        case KEY:   
            struct SKey* skey = (struct SKey*) malloc(sizeof(struct SKey));
            // skey->valor = (int) valor;
            skey->valor = (point*) valor;
            
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

no* TrocarCor(no* x, int versao) {
    // x->cor = !x->cor;
    Cor cor = ((struct SCor*) PegarModificacao(x, COR, versao))->valor;
    x = SalvarModificacao(x, COR, versao, (void*) !cor);
    
    no* x_esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;

    if (x_esq != NIL_PTR) {
        // x->esq->cor = !x->esq->cor;

        Cor x_esq_cor = ((struct SCor*) PegarModificacao(x_esq, COR, versao))->valor;
        no* new_x_esq = SalvarModificacao(x_esq, COR, versao, (void*) !x_esq_cor);

        if ( x_esq != new_x_esq ) {
            x = SalvarModificacao(x, LEFT, versao, new_x_esq);
        }
    }

    no* x_dir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
    
    if (x_dir != NIL_PTR) {
        // x->dir->cor = !x->dir->cor;

        Cor x_dir_cor = ((struct SCor*) PegarModificacao(x_dir, COR, versao))->valor;
        no* new_x_dir = SalvarModificacao(x_dir, COR, versao, (void*) !x_dir_cor);

        if ( x_dir != new_x_dir ) {
            x = SalvarModificacao(x, RIGHT, versao, new_x_dir);
        }
    }

    return x;
}

no* RotacaoESQ(no* x, int versao) {
    // no* z = x->dir;

    // x->dir = z->esq;
    // z->esq = x;
    // z->cor = x->cor;
    // x->cor = RUBRO;

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

no* RotacaoDIR(no* x, int versao) {
    // no* z = x->esq;

    // x->esq = z->dir;
    // z->dir = x;
    // z->cor = x->cor;
    // x->cor = RUBRO;

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

no* MoverEsquerda(no* x, int versao) {
    x = TrocarCor(x, versao);

    no* dir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
    Cor dirCor = ((struct SCor*) PegarModificacao(dir, COR, versao))->valor;
    no* dir_esq = ((struct SLeft*) PegarModificacao(dir, LEFT, versao))->valor;
    Cor dir_esqCor = ((struct SCor*) PegarModificacao(dir_esq, COR, versao))->valor;

    if ( dir_esqCor == RUBRO ) {
        // x->dir = RotacaoDIR(x->dir, versao);

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

        // int key = ((struct SKey*) PegarModificacao(z, KEY, versao))->valor;
        point* key = ((struct SKey*) PegarModificacao(z, KEY, versao))->valor;
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

// no* IncluirRecursivo(no* x, int valor, int versao) {
no* IncluirRecursivo(no* x, float valor, point* segmento, int versao) {
    if (x == NIL_PTR) {
        no* z = CriarNo(segmento);
        
        return z;
    }
    
    // int key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;
    point* key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;
    // if (valor < key) {
    float k1 = calcular_y(valor, segmento);
    float k2 = calcular_y(valor, key);

    if ( k1 == k2 ) {
        return x;
    }

    if ( k1 < k2 ) {
        // no* oldEsq = x->esq;
        no* oldEsq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
        no* newEsq = IncluirRecursivo(oldEsq, valor, segmento, versao);

        if (oldEsq != newEsq) {
            x = SalvarModificacao(x, LEFT, versao, newEsq);
        }
    }

    key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;

    k1 = calcular_y(valor, segmento);
    k2 = calcular_y(valor, key);

    // if (valor > key) {
    if ( k1 > k2 ) {
        // no* oldDir = x->dir;
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

    if ( esq_esq != NIL_PTR ) {
        esq_esqCor = ((struct SCor*) PegarModificacao(esq_esq, COR, versao))->valor;
    }

    if ( esqCor == RUBRO && esq_esq != NIL_PTR && esq_esqCor == RUBRO ) {
        x = RotacaoDIR(x, versao);
    }

    // se 2 filhos vermelhos trocam-se as cores
    if ( x != NIL_PTR ) {
        dir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
    }

    if ( x != NIL_PTR ) { 
        esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
    }
    
    if ( dir != NIL_PTR ) {
        dirCor = ((struct SCor*) PegarModificacao(dir, COR, versao))->valor;
    }

    if ( esq != NIL_PTR ) {
        esqCor = ((struct SCor*) PegarModificacao(esq, COR, versao))->valor;
    }

    if ( x != NIL_PTR && esq != NIL_PTR && esqCor == RUBRO && dir != NIL_PTR && dirCor == RUBRO ) {
        x = TrocarCor(x, versao);
    }

    return x;
}

int Incluir(Arn* T, float valor, point* segmento) {
    if (!Consultar(T, valor, segmento)) {
        T->raiz[T->ultima_versao] = IncluirRecursivo(T->raiz[T->ultima_versao], valor, segmento, T->ultima_versao);

        if ( T->raiz[T->ultima_versao] != NIL_PTR ) {
            // T->raiz[T->ultima_versao]->cor = NEGRO;

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

    if ( esq != NIL_PTR && esq_esq != NIL_PTR ) {
        esq_esqCor = ((struct SCor*) PegarModificacao(esq_esq, COR, versao))->valor;
    }

    if ( esq != NIL_PTR && esqCor == RUBRO && esq_esq != NIL_PTR && esq_esqCor == RUBRO ) {
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

no* RemoverMenor(no* x, int versao) {
    no* esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;

    if ( esq == NIL_PTR ) {
        // free(x);
        return NIL_PTR;
    }

    esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;    
    Cor esqCor = ((struct SCor*) PegarModificacao(esq, COR, versao))->valor;
    no* esq_esq = ((struct SLeft*) PegarModificacao(esq, LEFT, versao))->valor;
    Cor esq_esqCor;

    if ( esq_esq != NIL_PTR ) {
        esq_esqCor = ((struct SCor*) PegarModificacao(esq_esq, COR, versao))->valor;
    }

    if ( esqCor == NEGRO && esq_esqCor == NEGRO ) {
        x = MoverEsquerda(x, versao);
    }

    // x->esq = RemoverMenor(x->esq, versao);

    no* oldEsq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
    no* newEsq = RemoverMenor(oldEsq, versao);

    if (oldEsq != newEsq) {
        x = SalvarModificacao(x, LEFT, versao, newEsq);
    }
    
    return Balancear(x, versao);
}

no* ProcurarMenor(no* x, int versao) {
    no* z = x;
    // no* y = x->esq;
    no* y = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;

    while ( y != NIL_PTR ) {
        z = y;
        // y = y->esq;
        y = ((struct SLeft*) PegarModificacao(y, LEFT, versao))->valor;
    }
    
    return z;
}

no* RemoverRecursivo(no* x, float valor, point* segmento, int versao) {
    // int key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;
    point* key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;
    // if ( valor < key ) {
    if ( calcular_y(valor, segmento) < calcular_y(valor, key) ) {
        no* esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
        no* esq_esq = ((struct SLeft*) PegarModificacao(esq, LEFT, versao))->valor;
        Cor esqCor = ((struct SCor*) PegarModificacao(esq, COR, versao))->valor;
        Cor esq_esqCor;

        // if ( esq_esq != NIL_PTR ) {
        esq_esqCor = ((struct SCor*) PegarModificacao(esq_esq, COR, versao))->valor;
        // }

        if ( esqCor == NEGRO && esq_esqCor == NEGRO ) {
            x = MoverEsquerda(x, versao);
        }

        // x->esq = RemoverRecursivo(x->esq, valor, versao);
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

        // int key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;
        point* key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;
        no* dir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
        
        // if ( key == valor && dir == NIL_PTR ) {
        if ( calcular_y(valor, segmento) == calcular_y(valor, key) && dir == NIL_PTR ) {
            // free(x);
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

        // if ( key == valor ) {
        if ( calcular_y(valor, segmento) == calcular_y(valor, key) ) {
            // copia o menor
            no* y = ProcurarMenor(dir, versao);
            // int yKey = ((struct SKey*) PegarModificacao(y, KEY, versao))->valor;
            point* yKey = ((struct SKey*) PegarModificacao(y, KEY, versao))->valor;
            
            // x->key = y->key;
            x = SalvarModificacao(x, KEY, versao, yKey);
            
            // remove o menor
            // x->dir = RemoverMenor(dir, versao);

            no* oldDir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
            no* newDir = RemoverMenor(oldDir, versao);

            if (oldDir != newDir) {
                x = SalvarModificacao(x, RIGHT, versao, newDir);
            }
        }
        else {
            // x->dir = RemoverRecursivo(x->dir, valor, versao);

            no* oldDir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
            no* newDir = RemoverRecursivo(oldDir, valor, segmento, versao);

            if (oldDir != newDir) {
                x = SalvarModificacao(x, RIGHT, versao, newDir);
            }
        }
    }

    return Balancear(x, versao);
}


int Consultar(Arn* T, float valor, point* segmento) {
    if( T->raiz[T->ultima_versao] == NULL )
        return 0;

    no* x = T->raiz[T->ultima_versao];

    while( x != NIL_PTR ){
        // int key = ((struct SKey*) PegarModificacao(x, KEY, T->ultima_versao))->valor;
        point* key = ((struct SKey*) PegarModificacao(x, KEY, T->ultima_versao))->valor;

        // if( valor == key ) {
        if ( calcular_y(valor, segmento) == calcular_y(valor, key) ) {
            return 1;
        }
        
        no* dir = ((struct SRight*) PegarModificacao(x, RIGHT, T->ultima_versao))->valor;
        // int dirkey = ((struct SKey*) PegarModificacao(dir, KEY, T->ultima_versao))->valor;
        point* dirkey = ((struct SKey*) PegarModificacao(dir, KEY, T->ultima_versao))->valor;
        no* esq = ((struct SLeft*) PegarModificacao(x, LEFT, T->ultima_versao))->valor;

        // if( valor >= dirkey ) {
        if ( dirkey != NULL && calcular_y(valor, segmento) >= calcular_y(valor, dirkey) ) {
            x = dir;
        }
        else {
            x = esq;
        }
    }
    
    return 0;
}

int Remover(Arn* T, float valor, point* segmento) {
    if (Consultar(T, valor, segmento)) {
        T->raiz[T->ultima_versao] = RemoverRecursivo(T->raiz[T->ultima_versao], valor, segmento, T->ultima_versao);

        if ( T->raiz[T->ultima_versao] != NIL_PTR ) {
            // T->raiz[T->ultima_versao]->cor = NEGRO;

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

void PreOrdem(no* x, int nivel, float parent, char* d, int versao, float valor) {
    if( x != NIL_PTR ) {
        no* dir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
        no* esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
        Cor cor = ((struct SCor*) PegarModificacao(x, COR, versao))->valor;
        // int key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;
        point* key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;

        if(cor == RUBRO)
            printf("%f  Vermelho: H(%d) P(%f) (%s) \n", calcular_y(valor, key), nivel, parent, d);
        else
            printf("%f  Preto:    H(%d) P(%f) (%s) \n", calcular_y(valor, key), nivel, parent, d);

        PreOrdem(esq, nivel+1, calcular_y(valor, key), "<-", versao, valor);
        PreOrdem(dir, nivel+1, calcular_y(valor, key), "->", versao, valor);
    }
}

/*void testeDePersistencia() {
    Arn* T = CriarArvore();

    int numeros[15] = {20, 30, 10, 15, 50, 60, 7, 3, 2, 25, 40, 55, 70, 22, 56};

    Incluir(T, numeros[0]);
    Incluir(T, numeros[1]);
    Incluir(T, numeros[2]);
    Incluir(T, numeros[3]);
    Incluir(T, numeros[4]);
    Incluir(T, numeros[5]);
    Incluir(T, numeros[6]);
    Incluir(T, numeros[7]);
    Incluir(T, numeros[8]);

    // for (int i = 0; i < 15; i++) {
    //     Incluir(T, numeros[i]);
    // }

    // Exibe o resultado
    // PreOrdem(T->raiz, 0, (int) -INFINITY, "v");

    // printf("------------------------------\n");

    Remover(T, 50);
    Remover(T, 20);
    Remover(T, 3);
    Remover(T, 15);
    Remover(T, 30);
    Remover(T, 60);

    // Exibe o resultado
    printf("---------- 0 -------- \n");
    PreOrdem(T->raiz[0], 0, (int) -INFINITY, "v", 0);
    printf("---------- 1 -------- \n");
    PreOrdem(T->raiz[1], 0, (int) -INFINITY, "v", 1);
    printf("---------- 2 -------- \n");
    PreOrdem(T->raiz[2], 0, (int) -INFINITY, "v", 2);
    printf("---------- 3 -------- \n");
    PreOrdem(T->raiz[3], 0, (int) -INFINITY, "v", 3);
    printf("---------- 4 ------- \n");
    PreOrdem(T->raiz[4], 0, (int) -INFINITY, "v", 4);
    printf("---------- 5 ------- \n");
    PreOrdem(T->raiz[5], 0, (int) -INFINITY, "v", 5);
    printf("---------- 6 ------- \n");
    PreOrdem(T->raiz[6], 0, (int) -INFINITY, "v", 6);
    printf("---------- 7 ------- \n");
    PreOrdem(T->raiz[7], 0, (int) -INFINITY, "v", 7);
    printf("---------- 8 ------- \n");
    PreOrdem(T->raiz[8], 0, (int) -INFINITY, "v", 8);
    printf("---------- 9 ------- \n");
    PreOrdem(T->raiz[9], 0, (int) -INFINITY, "v", 9);
    printf("---------- 10 ------- \n");
    PreOrdem(T->raiz[10], 0, (int) -INFINITY, "v", 10);
    printf("---------- 11 ------- \n");
    PreOrdem(T->raiz[11], 0, (int) -INFINITY, "v", 11);
    printf("---------- 12 ------- \n");
    PreOrdem(T->raiz[12], 0, (int) -INFINITY, "v", 12);
    printf("---------- 13 ------- \n");
    PreOrdem(T->raiz[13], 0, (int) -INFINITY, "v", 13);
    printf("---------- 14 ------- \n");
    PreOrdem(T->raiz[14], 0, (int) -INFINITY, "v", 14);
}*/ 

#define N 5

int comparar(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

int remover_duplicatas(int *array, int tamanho) {
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

int main() {
    // testeDePersistencia();
    Arn* T = CriarArvore();

    /*
    tem y's repetidos devido acho que algum segmento errado
    */
    
    /*
    point* segmento;

    float pontos[N][4] = {
        {0, 2, 4, 3},
        {4, 3, 6, 5},
        {4, 3, 7, 2},
        {6, 1, 9, 0},
        {2, 2, 5, 2}
    };

    float x1, y1, x2, y2, m, c;

    for (int i=0; i<=N; i++) {
        x1 = pontos[i][0];
        y1 = pontos[i][1];
        x2 = pontos[i][2];
        y2 = pontos[i][3];

        m = (y2 - y1) / (x2 - x1);
        c = y1 - m * x1;

        // printf("%f * x + %f \n", m, c);

        segmento = (point*) malloc(sizeof(point));
        segmento->x = m;
        segmento->y = c;

        Incluir(T, 5, segmento);
    }

    PreOrdem(T->raiz[4], 0, (int) -INFINITY, "v", 4, 5);

    return 0;
    */























    point* segmento;

    float pontos[N][4] = {
        {0, 2, 4, 3},
        {4, 3, 6, 5},
        {4, 3, 7, 2},
        {6, 1, 9, 0},
        {2, 2, 5, 2}
    };

    int tx = N*2;
    float* xs = malloc(tx * sizeof(int));

    for (int i=0; i<N; i++) {
        xs[i] = pontos[i][0];
        xs[N+i] = pontos[i][2];
    }

    qsort(xs, tx, sizeof(int), comparar);

    tx = remover_duplicatas(xs, tx);

    printf("valores no eixo x \n");
    for (int i=0; i<tx; i++) {
        printf("%f \n", xs[i]);
    }

    printf("--------------------------------------------\n");

    int tamanho = 0;
    int* vetor = malloc(tamanho * sizeof(int));
    
    printf("novas retas \n");
    
    int nSegments = 0;
    int oldSegments = -1;
    float vSegmento = pontos[0][0];
    int intervalo = 1;
    
    int intervalos[6];

    /*float registrador[N][4] = {
        { 0.0, 0.0, 0.0, 0.0 },
        { 0.0, 0.0, 0.0, 0.0 },
        { 0.0, 0.0, 0.0, 0.0 },
        { 0.0, 0.0, 0.0, 0.0 },
        { 0.0, 0.0, 0.0, 0.0 }
    };*/
    int rg[] = { 0, 0, 0, 0 };

    for (int i=0; i<tx-1; i++) {
        printf("intervalo %d \n", intervalo);

        for (int j=0; j<N; j++) {
            float x1 = pontos[j][0];
            float y1 = pontos[j][1];
            float x2 = pontos[j][2];
            float y2 = pontos[j][3];

            float x = ( xs[i] + xs[i+1] )/2;
            float m = (y2 - y1) / (x2 - x1);
            float c = y1 - m * x1;
            float y = m * x + c;

            if ( x1 <= x && x2 >= x ) {
                rg[j] = 1;

                tamanho += 1;
                vetor = realloc(vetor, tamanho * sizeof(int));

                // if ( !(x1 == xs[i] && y1 == y) ) {
                    if ( vSegmento != x1 ) {
                        // printf("--------------------------------------------\n");
                        vSegmento = x1;
                    }

                    // printf("%f, %f, %f, %f \n", x1, y1, xs[i], y);

                    // pontos[j][0] = x; // xs[i];
                    // pontos[j][1] = y;

                    segmento = (point*) malloc(sizeof(point));
                    segmento->x = fixedFloat(m);
                    segmento->y = fixedFloat(c);

                    // float x =  xs[i]; //( xs[i] + xs[i+1] )/2;
                    printf("-------------- versão %d (%f) -------------------\n", nSegments, x);
                    printf("adicionar\n");

                    int code =  Incluir(T, x, segmento);
                    printf("+ m %f, c %f | %f %f %f %f \n ", segmento->x, segmento->y, x1, x2, y1, y2);

                    PreOrdem(T->raiz[nSegments], 0, (int) -INFINITY, "v", nSegments, x);

                    if (code) {
                        nSegments += 1;
                    }

                // }
            }
            else {
                rg[j] = 0;

                segmento = (point*) malloc(sizeof(point));
                segmento->x = fixedFloat(m);
                segmento->y = fixedFloat(c);

                printf("remover\n");

                printf("-------------- versão %d (%f) -------------------\n", nSegments, x);
                printf("- m %f, c %f \n", segmento->x, segmento->y);
                int code = Remover(T, x, segmento);

                // if ( oldSegments != nSegments ) {
                PreOrdem(T->raiz[nSegments], 0, (int) -INFINITY, "v", nSegments, x);
                // }

                if ( code ) {
                    nSegments += 1;
                }
            }

            oldSegments = nSegments;
        }

        // printf("--------------------------------------------\n");

        printf("\n\n segmentos presentes %d %d %d %d %d \n\n", rg[0], rg[1], rg[2], rg[3], rg[4]);
        // printf("nSegments %f", )
        intervalos[intervalo - 1] = T->ultima_versao-1;
        intervalo += 1;

        /*tamanho += 1;
        vetor = realloc(vetor, tamanho * sizeof(int));

        vetor[tamanho-1] = 0;*/
    }

    printf("%d, %d, %d, %d, %d, %d \n", intervalos[0], intervalos[1], intervalos[2], intervalos[3], intervalos[4], intervalos[5]);

    for (int i=0; i<6; i++) {
        float x = (xs[i] + xs[i+1])/2;
        printf("-------------------  intervalo %d -----------------------\n", i + 1);

        PreOrdem(T->raiz[intervalos[i]], 0, (int) -INFINITY, "v", intervalos[i], x);
    }

    /*int tamanho = 0;
    int* vetor = malloc(tamanho * sizeof(int));
    
    for (int i=0; i<5; i++) {
        tamanho += 1;
        vetor = realloc(vetor, tamanho * sizeof(int));

        vetor[tamanho-1] = 0;
    }

    for (int i=0; i<5; i++) {
        printf("%d \n", vetor[i]);
    }*/

    return 0;
}