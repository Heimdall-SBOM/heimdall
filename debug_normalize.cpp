#include <iostream>
#include <string>

int main() {
    std::string path = "/usr/lib/../lib64";
    std::cout << "Input: " << path << std::endl;
    
    size_t pos = path.find("/../");
    std::cout << "Found /../ at: " << pos << std::endl;
    
    if (pos != std::string::npos) {
        size_t prev_slash = path.rfind('/', pos - 1);
        std::cout << "Previous slash at: " << prev_slash << std::endl;
        
        if (prev_slash != std::string::npos) {
            std::string result = path.substr(0, prev_slash) + path.substr(pos + 3);
            std::cout << "Result: " << result << std::endl;
        }
    }
    
    return 0;
} 