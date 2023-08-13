# Embedded Systems Thermal Management Project
This repository contains the implementation of an embedded systems project focused on temperature monitoring and thermal management using the LM75BD temperature sensor. The project includes driver functions for sensor interfacing, a dynamic task system with queues, and an interrupt handler for overtemperature shutdown events. The goal is to ensure efficient temperature control and telemetry for CubeSat applications.

## Table of Contents
* Introduction
* Requirements
* Setup
* Usage
* Components
* Contributing
* License

# Introduction
The Embedded Systems Thermal Management Project aims to address the temperature monitoring and control needs of CubeSat applications. The project involves implementing various technical components, including I2C communication with the LM75BD temperature sensor, task synchronization using queues for telemetry, and an interrupt handler for overtemperature shutdown events.

## Requirements
To successfully build and run this project, you will need the following tools and dependencies:

Windows Subsystem for Linux 2 (WSL2)
CMake
Make
GCC
LM75BD Temperature Sensor Datasheet: Link
I2C Protocol Tutorial: Link
Setup
Clone this repository to your local machine.

Set up WSL2 on your Windows system: WSL2 Installation Guide

Install required build tools within WSL2:

```c
Copy code
sudo apt-get update
sudo apt-get install build-essential cmake
```
Follow the additional setup instructions provided in the project documentation.

## Usage
Follow these steps to build and test the project locally:

Navigate to the project directory in your WSL2 terminal.

Build the project and run unit tests:

```c
Copy code
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Test
cmake --build .
ctest --verbose
```
Run integration tests:

```c
Copy code
mkdir build_fw && cd build_fw
cmake .. -DCMAKE_BUILD_TYPE=FW
cmake --build .
./onboarding # Run the executable
```
Refer to the project documentation for additional usage instructions and details on interpreting test results.

## Components
This project consists of the following key components:

Sensor Driver Functions: Implement driver functions for interfacing with the LM75BD temperature sensor using the I2C protocol.

Thermal Management Task: Create a task that periodically measures temperature data using the driver functions and sends telemetry.

OS Interrupt Handler: Develop an interrupt handler for overtemperature shutdown (OS) events from the LM75BD sensor, ensuring efficient event handling and system responsiveness.

Queue System: Implement a dynamic queue system for efficient data exchange between tasks, enabling seamless temperature telemetry updates.

Console Printing: Utilize the printConsole function for generating log messages and monitoring program execution.

## Contributing
Contributions to this project are welcome! If you encounter issues or have suggestions for improvements, please create an issue or pull request in this repository.

## Style Guide

### Comments

#### Single Line Comments

Variable and function names should be descriptive enough to understand even without comments. Comments are needed to describe any complicated logic. You may use `//` or `/* */` for single line comments.

#### Function Comments

Function comments should exist in the .h file. For static functions, they should exist in the .c file. Function comments should follow the format shown below:
```c
/**
 * @brief Adds two numbers together
 *
 * @param num1 - The first number to add.
 * @param num2 - The second number to add.
 * @return Returns the sum of the two numbers.
 */
uint8_t addNumbers(uint8_t num1, uint8_t num2);
```

#### File Header Comments

- File comments are not required

#### Header Guard

We use `#pragma once` instead of include guards.

### ****Naming and typing conventions****

-   `variableNames` in camelCase
-   `functionNames()` in camelCase
-   `#define MACRO_NAME` in CAPITAL_SNAKE_CASE
-   `file_names` in snake_case
-   `type_defs` in snake_case with _t suffix
    -   Ex:
        ```c
        typedef struct {
          int a;
          int b;
        } struct_name_t
        ```
-   Import statements should be grouped in the following order:
    1.  Local imports (e.g. `#include "cc1120_driver.h`)
    2.  External library imports (e.g. `#include <os_semphr.h>`)
    3.  Standard library imports (e.g. `#include <stdint.h>`)

### ****General Rules****
Some of these rules don't apply in certain cases. Use your better judgement. To learn more about these rules, research NASA's Power of 10.

1. Avoid complex flow constructs, such as [goto](https://en.wikipedia.org/wiki/Goto) and [recursion](https://en.wikipedia.org/wiki/Recursion_(computer_science)).
2. All loops must have fixed bounds. This prevents runaway code.
3. Avoid [heap memory allocation](https://en.wikipedia.org/wiki/Memory_management#DYNAMIC).
4. Use an average of two [runtime assertions](https://en.wikipedia.org/wiki/Assertion_(software_development)#Assertions_for_run-time_checking) per function.
5. Restrict the scope of data to the smallest possible.
6. Check the return value of all non-void functions, or cast to void to indicate the return value is useless.
7. Limit pointer use to a single [dereference](https://en.wikipedia.org/wiki/Dereference_operator), and do not use [function pointers](https://en.wikipedia.org/wiki/Function_pointer).
8. Compile with all possible warnings active; all warnings should then be addressed before release of the software.
9. Use the preprocessor sparingly
10. Restrict functions to a single printed page
