#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    float a;
    float b;
} point;

float segmento_de_reta(float x, point* r1) {
    return r1->a * x + r1->b;
}

typedef struct
{
    float a;
    float b;

    float (*key)(float);
} reta;

int main() {
    reta* r1 = (reta*) malloc(sizeof(reta));
    r1->a = 1;
    r1->b = 1;

    float y = segmento_de_reta(30, r1);
    // printf("%f", y);

    int *vetor;

    int tamanho = 1;
    vetor = (int *)malloc(tamanho * sizeof(int));

    for (int i=0; i<4; i++) {
        
    }

    return 0;
}