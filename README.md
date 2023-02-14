# OBC Firmware Onboarding Challenge

Welcome to Orbital's OBC Firmware Onboarding Challenge! Please visit [this Notion doc](https://www.notion.so/uworbital/Firmware-Onboarding-Challenge-b208ef6d62cd436a8e738dc99f6d2c4d) for the challenge instructions. Remember to follow our style guide which is written below.

## Style Guide

### Comments

#### Single Line Comments

Variable and function names should be descriptive enough to understand even without comments. Comments are needed to describe any complicated logic. You may use `//` or `/* */` for single line comments. 

#### Function Comments

Function comments should exist in both the .h and .c files optimally, but at minimum they should be available in the .h files. Comments should follow the format shown below:
```c
/**
 * @brief Adds two numbers together
 * 
 * @param num1 - The first number to add.
 * @param num2 - The second number to add.
 * @return uint8_t - Returns the sum of of the two numbers.
 */
uint8_t add_numbers(uint8_t num1, uint8_t num2);
```

#### File Header Comments

- File comments are not required

#### Header Guard

- The symbol name should have the form `<PATH>_<FILE>_H_`

For example, if the file is `abc/xyz/foo.h`, then the header guard should be
```c
#ifndef ABC_XYZ_FOO_H_
#define ABC_XYZ_FOO_H_
...
#endif
```

### ****Naming and typing conventions****

-   `variableNames` in camelCase
-   `functionNames()` in camelCase
-   `CONSTANT_NAMES` in CAPITAL_SNAKE_CASE
-   `file_names` in snake_case
-   `type_defs` in snake_case with _t suffix
    -   Ex: 
        ```c
        typedef struct {
            int a;
            int b;
        } struct_name_t
        ```
-   4 spaces per level of indentation
-   Use spaces before opening brackets for conditionals and loops (e.g. `if ()` and `while ()`), but not for function calls (i.e. `my_func()`).
-   Operators:
    -   No spaces around `*`, `/`, `%`, `!`
    -   One space on either side of `=`, `==`, `+`, `-`, `+=`, `-=`, etc
    -   One space after every comma `my_func(var1, var2, var3)`
-   Import statments should be grouped in the following order:
    1.  Local imports (e.g. `#include "cc1120_driver.h`)
    2.  External library imports (e.g. `#include <semphr.h>`)
    3.  Standard library imports (e.g. `#include <stdint.h>`)
-   160 character limit per line (not a hard limit, use common sense)
-   Hanging indents should be aligned to delimeter:

```c
myFunction(hasToo,
            many, variables)
```

### ****General Rules****

1. Avoid complex flow constructs, such as [goto](https://en.wikipedia.org/wiki/Goto) and [recursion](https://en.wikipedia.org/wiki/Recursion_(computer_science)).
2. All loops must have fixed bounds. This prevents runaway code.
3. Avoid [heap memory allocation](https://en.wikipedia.org/wiki/Memory_management#DYNAMIC).
4. Use an average of two [runtime assertions](https://en.wikipedia.org/wiki/Assertion_(software_development)#Assertions_for_run-time_checking) per function.
5. Restrict the scope of data to the smallest possible.
6. Check the return value of all non-void functions, or cast to void to indicate the return value is useless.
7. Limit pointer use to a single [dereference](https://en.wikipedia.org/wiki/Dereference_operator), and do not use [function pointers](https://en.wikipedia.org/wiki/Function_pointer).
8. Compile with all possible warnings active; all warnings should then be addressed before release of the software.
