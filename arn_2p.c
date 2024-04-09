#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "arn_2p.h"

#define MAX_OP 20
#define MAX_QM 2

/////// PERSISTENCIA //////

struct SKey { int valor; };
struct SLeft { no* valor; };
struct SRight { no* valor; };
struct SCor { Cor valor; };

typedef struct {
    int versao;
    Tipo tipo;
    void *ref;
} Mod;

///////////////////////////

struct no {
    int        key;
    struct no* esq;
    struct no* dir;
    Cor        cor;

    Mod*       mod[MAX_QM];
    int        qmods;
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

no* CriarNo(int valor) {
    no* z = (no*) malloc(sizeof(no));
    z->key = valor;
    z->esq = z->dir = NIL_PTR;
    z->cor = RUBRO;
    return z;
}

void* CriarReferencia(Tipo tipo, void* valor) {
    void* ref = NULL;

    switch (tipo) {
        case KEY:   
            struct SKey* skey = (struct SKey*) malloc(sizeof(struct SKey));
            skey->valor = (int) valor;
            
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
        no* y = CriarNo(-1);

        int key = ((struct SKey*) PegarModificacao(z, KEY, versao))->valor;
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

no* IncluirRecursivo(no* x, int valor, int versao) {
    if (x == NIL_PTR) {
        no* z = CriarNo(valor);
        
        return z;
    }
    
    int key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;
    if (valor < key) {
        // no* oldEsq = x->esq;
        no* oldEsq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
        no* newEsq = IncluirRecursivo(oldEsq, valor, versao);

        if (oldEsq != newEsq) {
            x = SalvarModificacao(x, LEFT, versao, newEsq);
        }
    }
    
    key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;
    if (valor > key) {
        // no* oldDir = x->dir;
        no* oldDir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
        no* newDir = IncluirRecursivo(oldDir, valor, versao);

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

void Incluir(Arn* T, int valor) {
    T->raiz[T->ultima_versao] = IncluirRecursivo(T->raiz[T->ultima_versao], valor, T->ultima_versao);

    if ( T->raiz[T->ultima_versao] != NIL_PTR ) {
        // T->raiz[T->ultima_versao]->cor = NEGRO;

        T->raiz[T->ultima_versao] = SalvarModificacao(T->raiz[T->ultima_versao], COR, T->ultima_versao, (void*) NEGRO);
    }

    T->raiz[T->ultima_versao + 1] = T->raiz[T->ultima_versao];

    T->ultima_versao += 1;
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

no* RemoverRecursivo(no* x, int valor, int versao) {
    int key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;
    if ( valor < key ) {
        no* esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
        no* esq_esq = ((struct SLeft*) PegarModificacao(esq, LEFT, versao))->valor;
        Cor esqCor = ((struct SCor*) PegarModificacao(esq, COR, versao))->valor;
        Cor esq_esqCor;

        if ( esq_esq != NIL_PTR ) {
            esq_esqCor = ((struct SCor*) PegarModificacao(esq_esq, COR, versao))->valor;
        }

        if ( esqCor == NEGRO && esq_esqCor == NEGRO ) {
            x = MoverEsquerda(x, versao);
        }

        // x->esq = RemoverRecursivo(x->esq, valor, versao);
        no* oldEsq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
        no* newEsq = RemoverRecursivo(oldEsq, valor, versao);

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

        int key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;
        no* dir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
        
        if ( key == valor && dir == NIL_PTR ) {
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

        if ( key == valor ) {
            // copia o menor
            no* y = ProcurarMenor(dir, versao);
            int yKey = ((struct SKey*) PegarModificacao(y, KEY, versao))->valor;
            
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
            no* newDir = RemoverRecursivo(oldDir, valor, versao);

            if (oldDir != newDir) {
                x = SalvarModificacao(x, RIGHT, versao, newDir);
            }
        }
    }

    return Balancear(x, versao);
}


int Consultar(Arn* T, int valor) {
    if( T->raiz[T->ultima_versao] == NULL )
        return 0;

    no* x = T->raiz[T->ultima_versao];

    while( x != NIL_PTR ){
        int key = ((struct SKey*) PegarModificacao(x, KEY, T->ultima_versao))->valor;

        if( valor == key ) {
            return 1;
        }
        
        no* dir = ((struct SRight*) PegarModificacao(x, RIGHT, T->ultima_versao))->valor;
        int dirkey = ((struct SKey*) PegarModificacao(dir, KEY, T->ultima_versao))->valor;
        no* esq = ((struct SLeft*) PegarModificacao(x, LEFT, T->ultima_versao))->valor;

        if( valor >= dirkey ) {
            x = dir;
        }
        else {
            x = esq;
        }
    }
    
    return 0;
}

void Remover(Arn* T, int valor) {
    if (Consultar(T, valor)) {
        T->raiz[T->ultima_versao] = RemoverRecursivo(T->raiz[T->ultima_versao], valor, T->ultima_versao);

        if ( T->raiz[T->ultima_versao] != NIL_PTR ) {
            T->raiz[T->ultima_versao]->cor = NEGRO;
        }

        T->raiz[T->ultima_versao + 1] = T->raiz[T->ultima_versao];

        T->ultima_versao += 1;
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

void PreOrdem(no* x, int nivel, int parent, char* d, int versao) {
    if( x != NIL_PTR ) {
        no* dir = ((struct SRight*) PegarModificacao(x, RIGHT, versao))->valor;
        no* esq = ((struct SLeft*) PegarModificacao(x, LEFT, versao))->valor;
        Cor cor = ((struct SCor*) PegarModificacao(x, COR, versao))->valor;
        int key = ((struct SKey*) PegarModificacao(x, KEY, versao))->valor;

        if(cor == RUBRO)
            printf("%d  Vermelho: H(%d) P(%d) (%s) \n", key, nivel, parent, d);
        else
            printf("%d  Preto:    H(%d) P(%d) (%s) \n", key, nivel, parent, d);

        PreOrdem(esq, nivel+1, key, "<-", versao);
        PreOrdem(dir, nivel+1, key, "->", versao);
    }
}

int main() {
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

    /*for (int i = 0; i < 15; i++) {
        Incluir(T, numeros[i]);
    }*/

    // Exibe o resultado
    // PreOrdem(T->raiz, 0, (int) -INFINITY, "v");

    // printf("------------------------------\n");

    // Remover(T, 50);
    // Remover(T, 20);
    // Remover(T, 3);
    // Remover(T, 15);

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
    // printf("---------- 12 ------- \n");
    // PreOrdem(T->raiz[12], 0, (int) -INFINITY, "v", 12);
    // printf("---------- 13 ------- \n");
    // PreOrdem(T->raiz[13], 0, (int) -INFINITY, "v", 13);

    return 0;
}