#pragma once

#include "ComponentInfo.hpp"
#include <unordered_map>
#include <memory>

namespace heimdall {

class SBOMGenerator {
public:
    SBOMGenerator();
    ~SBOMGenerator();
    
    void processComponent(const ComponentInfo& component);
    void generateSBOM();
    void setOutputPath(const std::string& path);
    void setFormat(const std::string& format);
    
    size_t getComponentCount() const;
    bool hasComponent(const std::string& name) const;
    void printStatistics() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace heimdall
