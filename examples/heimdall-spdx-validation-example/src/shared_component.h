#pragma once

#include <string>

/**
 * @brief Shared component for SPDX testing
 */
class SharedComponent {
public:
    SharedComponent();
    ~SharedComponent();
    
    /**
     * @brief Process shared data
     * @param data Input data to process
     * @return Processed result
     */
    std::string process(const std::string& data);
    
    /**
     * @brief Get component version
     * @return Version string
     */
    std::string getVersion() const;

private:
    std::string version;
}; 