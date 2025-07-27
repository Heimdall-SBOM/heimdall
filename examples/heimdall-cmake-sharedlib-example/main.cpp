#include <iostream>
#include "shared_greeter.h"

int main()
{
   std::cout << shared_greet("SharedLib User") << std::endl;
   return 0;
}