/*
Copyright 2025 The Heimdall Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
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