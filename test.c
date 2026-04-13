#include <stdio.h>
#include <stdint.h>

uint16_t q3(uint8_t x, uint8_t y) {
    uint16_t result_One = x^1;
    uint8_t a=128;
    printf("%d", a&x);
    printf("\n");
    while ((a&x)!=a){
        a>>=1;
    }
    result_One^=a;

    uint16_t result_Final=result_One<<8;
    result_Final+=y;

    return result_Final;
}

int main(){
    uint16_t result = q3(0b10010010, 0b01100101);

    //scanf("%d", &result);

    printf("%d", result);
}
