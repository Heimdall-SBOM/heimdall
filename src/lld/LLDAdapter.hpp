#pragma once

#include <memory>
#include <string>

namespace heimdall {

class LLDAdapter {
public:
    LLDAdapter();
    ~LLDAdapter();
    
    void initialize();
    void processInputFile(const std::string& filePath);
    void finalize();
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace heimdall
