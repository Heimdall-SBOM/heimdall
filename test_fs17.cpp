#include <iostream>
#include <filesystem>

int main() {
    std::filesystem::path p{"test.txt"};
    std::cout << "Path: " << p.string() << std::endl;
    return 0;
} 