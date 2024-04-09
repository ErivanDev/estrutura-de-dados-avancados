#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum { NEGRO, RUBRO } Cor;

typedef struct {
    int        key;
    struct no* esq;
    struct no* dir;
    Cor        cor;
} no;

struct Arn {
    struct no* raiz;
};

struct Key { const char* valor; };
struct Left { no* valor; };
struct Right { no* valor; };
struct Cor { int valor; };

typedef enum { KEY, LEFT, RIGHT, COR } Tipo;

typedef struct {
    int versao;
    Tipo tipo;
    void *ref;
} Mod;

int main() {
    Mod* d = (Mod*) malloc(sizeof(Mod));

    struct Key* s1 = (struct Key*)malloc(sizeof(struct Key));
    struct Cor* s2 = (struct Cor*)malloc(sizeof(struct Cor));

    s1->valor = "erivan";
    s2->valor = 10;

    // d->ref = malloc(sizeof(struct Key));
    // *((struct Key *) d->ref) = *s1;
    
    d->ref = 10; // s1;

    d->tipo = KEY;

    /*d->valor = malloc(sizeof(struct Cor));
    *((struct Cor *) d->valor) = *s2;

    d->tipo = COR;*/

    if (d->tipo == KEY) {
        printf("%d \n", (int) d->ref);
    } else if (d->tipo == COR) {
        printf("%d \n", ((struct Cor *)d->ref)->valor);
    }
}