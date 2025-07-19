#include "shared_component.h"
#include <iostream>

SharedComponent::SharedComponent() : version("10") {
    std::cout << "[SharedComponent] Initialized v" << version << std::endl;
}

SharedComponent::~SharedComponent() {
    std::cout << "[SharedComponent] Destroyed" << std::endl;
}

std::string SharedComponent::process(const std::string& data) {
    std::string result = "[SHARED] " + data;
    return result;
}

std::string SharedComponent::getVersion() const {
    return version;
} 