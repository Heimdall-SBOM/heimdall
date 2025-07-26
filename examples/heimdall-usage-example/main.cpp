#include <iostream>
#include <string>
#include "math.h"
#include "utils.h"

int main()
{
  std::cout << "Hello from Heimdall example!" << std::endl;

  // String operations
  std::string message = "Hello, World!";
  std::cout << "String: " << message << std::endl;
  std::cout << "Length: " << getStringLength(message) << std::endl;

  // Math operations
  int a = 2, b = 3;
  std::cout << "Math: " << a << " + " << b << " = " << add(a, b) << std::endl;

  int c = 10, d = 5;
  std::cout << "Math: " << c << " * " << d << " = " << multiply(c, d) << std::endl;

  return 0;
}