#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//-------------------------------------------------------------------------
// Question 0
// Include the challenge.h header file
//-------------------------------------------------------------------------
#include "challenge.h"

//-------------------------------------------------------------------------
// Question 1
// Declare two global variables. Both are integers named `q1A` and `q1B`,
// respectively. The value of `q1A` should be initialized to 0 and the value
// of `q1B`should be initialized to 1.
//-------------------------------------------------------------------------
int q1A = 0;
int q1B = 1;

//-------------------------------------------------------------------------
// Question 2
// Declare a global variable for an integer array of size 10. The name of
// the array should be `q2Array`. The size should be defined by a macro
// named `Q2_ARRAY_SIZE`.
//-------------------------------------------------------------------------
#define Q2_ARRAY_SIZE 10
int q2Array[Q2_ARRAY_SIZE];

//-------------------------------------------------------------------------
// Question 3
// Complete the following function. The function should flip the most
// significant bit and the least significant bit of the input byte `x`.
// The function should then return the y value appended to the new x value.
//
// Example: x = 0b10010010, y = 0b01100101
//          x becomes 0b00010011
//          The function should return 0b0001001101100101
//-------------------------------------------------------------------------
uint16_t q3(uint8_t x, uint8_t y)
{

    x ^= 0x80;
    x = x ^ 0x01;
    return (((uint16_t)x << 8) | y);
}

//-------------------------------------------------------------------------
// Question 4
// Fix all the issues with the following function, `q4`.
// The function should return the sum of all the elements in the array
// pointed to by `array`. The length of the array is given by the parameter
// `arrayLength`. Deal with all possible errors. It should return -1 if any
// errors occur.
//
// Note: The array contains 8-bit unsigned integers.
//-------------------------------------------------------------------------
int32_t q4(uint8_t *array, uint32_t arrayLength)
{
    if (array == NULL)
        return -1;

    int32_t sum = 0;
    for (uint8_t i = 0; i <= arrayLength; i++)
    {
        sum += array[i];
    }
    return sum;
}

//-------------------------------------------------------------------------
// Question 5
// Define a type called `q5_t` that is a union with the following fields:
// - uint32_t a
// - uint16_t b
//-------------------------------------------------------------------------

typedef union
{
    uint32_t a;
    uint16_t b;
} q5_t;

//-------------------------------------------------------------------------
// Question 6
// Define a type called `q6_t` that is a structure with the following
// members:
// - uint32_t x
// - uint16_t y
//-------------------------------------------------------------------------
typedef struct
{
    uint32_t x;
    uint16_t y;
} q6_t;

//-------------------------------------------------------------------------
// Question 7
// Define a type called `error_t` that is an enum with the following
// values:
// - SUCCESS = 0
// - FAIL = 1
//-------------------------------------------------------------------------
typedef enum
{
    SUCCESS = 0,
    FAIL = 1
} error_t;
//-------------------------------------------------------------------------
// Question 8
// Define a macro called `MULTIPLY` that takes two parameters and multiplies
// them together. The macro should return the result.
//-------------------------------------------------------------------------
#define MULTIPLY(a, b) ((a) * (b))

//-------------------------------------------------------------------------
// Question 9
// Complete the following function. The function swaps the values of two
// integers pointed to by `a` and `b`. The function should return 0 if the
// swap was successful and -1 if the swap failed.
//
// Example:
// int x = 5, y = 10;
// q9(&x, &y); // returns 0
// Now, x = 10 and y = 5 trial
//-------------------------------------------------------------------------
int q9(int *a, int *b)
{
    if (a == NULL || b == NULL)
        return -1;

    int temp = *a;
    *a = *b;
    *b = temp;
    return 0;
}

//-------------------------------------------------------------------------
// Question 10
// Complete the following function. The function should swap the values of
// the `a` and `b` members in the `q10_t` structure pointed to by `q10`. Use
// the `q9` function you created. The `q10` function should return SUCCESS
// if the swap was successful, and FAIL if the swap failed.
//
// Note: The error_t type is defined in question 7.
//-------------------------------------------------------------------------
typedef struct
{
    int a;
    int b;
} q10_t;

error_t q10(q10_t *q10)
{
    if (q10 == NULL)
        return FAIL;
    int swapResult = q9(&q10->a, &q10->b);
    return (swapResult == 0) ? SUCCESS : FAIL;
}

//-------------------------------------------------------------------------
// Question 11
// Complete the following function. The function should copy over the values
// in the array of struct a into struct b at an offset of 1, so the value stored
// at index 0 in the array of struct a should be copied into index 1 of the
// array in struct b and so on. This should be done without a loop. The `q11`
// function should return SUCCESS if the copy was successful, and FAIL if
// the copy failed.
//
// Note: The error_t type is defined in question 7.
//-------------------------------------------------------------------------
typedef struct
{
    uint16_t array[50];
} q11_a_t;

typedef struct
{
    uint16_t array[51];
} q11_b_t;

error_t q11(q11_a_t *a, q11_b_t *b)
{
}

//-------------------------------------------------------------------------
// Question 12
// Define a macro called `MIN` that takes two parameters and finds the
// lesser value of the 2. The macro should return the result.
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// Question 13
// Complete the following function. The function should return
// the minimum of the addresses pointed to by ptr1 and ptr 2 incremented
// by 5, so if ptr1 was pointing to 0x00000004 and ptr2 was pointing to
// 0x00000006 the function should return a void pointer pointing to
// 0x00000009 or -1 if there is an error.
//-------------------------------------------------------------------------

void *q13(uint32_t *ptr1, uint16_t *ptr2)
{
}
//-------------------------------------------------------------------------
// The following function is used to test your code. Do not remove any
// existing code. You may add additional tests if you wish.
//-------------------------------------------------------------------------
int main(void)
{
    // Question 0 Test
    ASSERT(0 == 0);

    // Question 1 Test
    ASSERT(q1A == 0);
    ASSERT(q1B == 1);

    q1A = -1;
    ASSERT(q1A == -1);

    q1B = -3;
    ASSERT(q1B == -3);

    q1A = 0;
    q1B = 1;

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
    error_t error = SUCCESS;
    ASSERT(error == SUCCESS);
    error = FAIL;
    ASSERT(error == FAIL);

    // Question 8 Test
    ASSERT(MULTIPLY(1, 2) == 2);
    ASSERT(MULTIPLY(-1, 1) == -1);
    ASSERT(MULTIPLY(2.0, 2.0) == 4.0);
    ASSERT(MULTIPLY(1 + 3, 4 + 6) == 40);

    // Question 9 Test
    int x = 5, y = 10;
    ASSERT(q9(&x, &y) == 0);
    ASSERT(x == 10);
    ASSERT(y == 5);
    ASSERT(q9(NULL, &y) == -1);
    ASSERT(q9(&x, NULL) == -1);
    ASSERT(q9(NULL, NULL) == -1);

    // Question 10 Test
    q10_t q10Test = {.a = 5, .b = 10};
    ASSERT(q10(&q10Test) == SUCCESS);
    ASSERT(q10Test.a == 10);
    ASSERT(q10Test.b == 5);
    ASSERT(q10(NULL) == FAIL);

    q11_a_t a;
    q11_b_t b;
    for (uint8_t i = 0; i < 50; ++i)
    {
        a.array[i] = i;
    }
    ASSERT(q11(&a, &b) == SUCCESS);
    ASSERT(memcmp(a.array, b.array + 1, 50) == 0);
    ASSERT(q11(&a, NULL) == FAIL);
    ASSERT(q11(NULL, &b));

    ASSERT(MIN(2, 4) == 2);
    ASSERT(MIN(2.1, 2.2) == 2.1);
    ASSERT(MIN(52, 2) == 2);
    ASSERT(MIN(5, 5) == 5);

    uint32_t *ptr1 = (uint32_t *)0x10;
    uint16_t *ptr2 = (uint16_t *)0x12;
    ASSERT(q13(ptr1, ptr2) == (void *)0x15);
    ptr1 = (uint32_t *)0x3129;
    ptr2 = (uint16_t *)0x3124;
    ASSERT(q13(ptr1, ptr2) == (void *)0x3129);

    return 0;
}

/*
#include <stdint.h>


#define Q2_ARRAY_SIZE 10


int q2Array[Q2_ARRAY_SIZE];


uint16_t q3(uint8_t x, uint8_t y)
{
    x ^= 0x80; // Flipping the most significant bit of x
    x ^= 0x01; // Flipping the least significant bit of x
    return ((uint16_t)x << 8) | y;
}

/////////////////////////////////////////////////////////////////////
int32_t q4(uint8_t *array, uint32_t arrayLength)
{
    if (array == NULL)
        return -1;


    int32_t sum = 0;
    for (uint32_t i = 0; i < arrayLength; i++)
    {
        sum += array[i];
    }
    return sum;
}
/////////////////////////////////////////////////////////////////////
typedef union
{
    uint32_t a;
    uint16_t b;
} q5_t;

/////////////////////////////////////////////////////////////////////

typedef struct
{
    uint32_t x;
    uint16_t y;
} q6_t;

/////////////////////////////////////////////////////////////////////

typedef enum
{
    SUCCESS = 0,
    FAIL = 1
} error_t;

/////////////////////////////////////////////////////////////////////

#define MULTIPLY(a, b) ((a) * (b))


int q9(int *a, int *b)
{
    if (a == NULL || b == NULL)
        return -1;


    int temp = *a;
    *a = *b;
    *b = temp;
    return 0;
}