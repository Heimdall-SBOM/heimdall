#include <stdio.h>

// Test function 1
int test_function1(int x)
{
    return x * 2;
}

// Test function 2
void test_function2(const char* message)
{
    printf("Message: %s\n", message);
}

// Test function 3
double test_function3(double a, double b)
{
    return a + b;
}

// Global variable
int global_test_var = 42;

// Weak symbol
__attribute__((weak)) int weak_test_symbol = 100; 