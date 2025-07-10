#include "src/compat/compatibility.hpp"
#include <iostream>
#include <string>
#include <optional>
#include <format>

int main() {
    std::optional<int> opt;
    opt = 42;
    std::cout << "opt has value: " << opt.has_value() << ", value: " << opt.value() << std::endl;
    std::string s = "hello";
    std::cout << s << std::endl;
    std::string formatted = std::format("Value: {}", opt.value());
    std::cout << formatted << std::endl;
    return 0;
} 