#include "processor.h"
#include <iostream>

Processor::Processor() : processedCount(0) {
    std::cout << "[Processor] Initialized" << std::endl;
}

Processor::~Processor() {
    std::cout << "[Processor] Destroyed" << std::endl;
}

std::string Processor::processData(const std::string& data) {
    processedCount++;
    
    std::string result = "[PROCESSED] " + data;
    return result;
}

int Processor::getProcessedCount() const {
    return processedCount;
} 