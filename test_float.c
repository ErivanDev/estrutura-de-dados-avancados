#include <stdio.h>

float fixedFloat(float valor) {
    float rounded_result = (int)(valor * 10000.0) / 10000.0;
    return rounded_result;
}

int main() {
    float numero = 3.14159;
    float numeroFixo = fixedFloat(numero);
    
    printf("Número com duas casas decimais: %f\n", numeroFixo);
    
    return 0;
}