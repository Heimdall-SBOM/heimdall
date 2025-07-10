#include <iostream>
#include <string>
#include "lib/greeter.h"

int main() {
    std::string message = "Hello from Heimdall CMake Module Example!";
    std::cout << message << std::endl;
    std::cout << get_greeting("CMake User") << std::endl;
    return 0;
} 