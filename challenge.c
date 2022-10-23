#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

//-------------------------------------------------------------------------
// Question 1
// Include the challenge.h header file
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
// Question 2
// Declare a global variable for an integer array of size 10. The name of
// the array should be `q2Array`. The size should be defined by a macro
// named `Q2_ARRAY_SIZE`.
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
// Question 3
// Complete the following function. The function should flip the most 
// significant bit and the least significant bit of the input byte `x`.
// The function should return the y value appended to the new x value.
// 
// Example: x = 0b10010010, y = 0b01100101
//          x becomes 0b00010011
//          The function should return 0b0001001101100101
//-------------------------------------------------------------------------
uint16_t q3(uint8_t x, uint8_t y) {

}

//-------------------------------------------------------------------------
// Question 4
// Fix all the problems with the following function
// The function should return the sum of all the elements in the array.
// Deal with all possible errors. It should return -1 if any errors occur.
//-------------------------------------------------------------------------
int32_t q4(uint8_t * array, uint32_t arrayLength) {
    for (uint8_t i = 0; i <= arrayLength; i++) {
        int32_t sum = 0;
        sum += array[i];
    }
}

//-------------------------------------------------------------------------
// Question 5
// Define a type called `q5_t` that is a union with the following fields:
// - uint32_t a
// - uint16_t b
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
// Question 6
// Define a type called `q6_t` that is a structure with the following
// members:
// - uint32_t x
// - uint16_t y
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
// Question 7
// Define a type called `state_t` that is an enum with the following
// possible values:
// - STATE_A = 0
// - STATE_B = 1
// - STATE_C = 2
// - FAIL_STATE = 3
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
// Question 8
// Define a macro called `MULTIPLY` that takes two parameters and multiplies
// them together. The macro should return the result.
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
// The following function is used to test your code. Do not remove any 
// existing code. You may add additional code if you wish.
//-------------------------------------------------------------------------
int main(void) {
    // Question 1 Test
    ASSERT(0 == 0);

    // Question 2 Test
    ASSERT(Q2_ARRAY_SIZE == 10);
    ASSERT(q2Array != NULL);

    q2Array[0] = 1;
    ASSERT(q2Array[0] == 1);

    q2Array[1] = -1;
    ASSERT(q2Array[1] == -1);

    // Question 3 Test
    ASSERT(q3(0b10010010, 0b01100101) == 0b0001001101100101);
    ASSERT(q3(3, 3) == 33283);
    ASSERT(q3(0x0, 0x0) == 0x8100);

    // Question 4 Test
    uint8_t smallArray[5] = {1, 2, 3, 4, 5};
    uint8_t largeArray[1000] = {1};
    ASSERT(q4(smallArray, 5) == 15);
    ASSERT(q4(smallArray, 0) == 0);
    ASSERT(q4(NULL, 10) == -1);
    ASSERT(q4(largeArray, 1000) == 1);
    
    // Question 5 Test
    q5_t q5 = {.a = 0x01020304};
    ASSERT(q5.a == 0x01020304);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    ASSERT(q5.b == 0x0304);
#else
    ASSERT(q5.b == 0x0102);
#endif

    // Question 6 Test
    q6_t q6 = {.x = 0x01020304, .y = 0x0506};
    ASSERT(q6.x == 0x01020304);
    ASSERT(q6.y == 0x0506);

    // Question 7 Test
    state_t state = STATE_A;
    ASSERT(state == 0);
    state = STATE_B;
    ASSERT(state == 1);
    state = STATE_C;
    ASSERT(state == 2);
    state = FAIL_STATE;
    ASSERT(state == 3);
    
    // Question 8 Test
    ASSERT(MULTIPLY(1, 2) == 2);
    ASSERT(MULTIPLY(-1, 1) == -1);
    ASSERT(MULTIPLY(2.0, 2.0) == 4.0);
    ASSERT(MULTIPLY(1 + 3, 4 + 6) == 40);

    return 0;
}