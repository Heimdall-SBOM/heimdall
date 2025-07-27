#include <iostream>
#include <vector>
#include <string>
#include "../src/common/SBOMComparator.hpp"

using namespace heimdall;

int main() {
    std::cout << "=== BOM Reference and Dependencies Demo ===\n\n";

    // Create components with BOM references
    SBOMComponent libA{"libA", "libA-1.0.0", "libA", "1.0.0", "library", "pkg:generic/libA@1.0.0", "MIT", {}, {}};
    SBOMComponent libB{"libB", "libB-2.0.0", "libB", "2.0.0", "library", "pkg:generic/libB@2.0.0", "Apache-2.0", {}, {}};
    SBOMComponent app{"app", "app-1.0.0", "myapp", "1.0.0", "application", "pkg:generic/myapp@1.0.0", "GPL-3.0", {}, {}};

    // Set up dependencies using BOM references
    libA.dependencies = {};  // libA has no dependencies
    libB.dependencies = {"libA-1.0.0"};  // libB depends on libA
    app.dependencies = {"libA-1.0.0", "libB-2.0.0"};  // app depends on both libA and libB

    std::vector<SBOMComponent> components = {libA, libB, app};

    // Display component information
    std::cout << "Components:\n";
    for (const auto& comp : components) {
        std::cout << "  - " << comp.name << " v" << comp.version << " (BOM-Ref: " << comp.bomRef << ")\n";
        if (!comp.dependencies.empty()) {
            std::cout << "    Dependencies: ";
            for (size_t i = 0; i < comp.dependencies.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << comp.dependencies[i];
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }

    // Display dependency relationships
    std::cout << "Dependency Relationships:\n";
    for (const auto& comp : components) {
        if (!comp.dependencies.empty()) {
            std::cout << "  " << comp.bomRef << " depends on: ";
            for (size_t i = 0; i < comp.dependencies.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << comp.dependencies[i];
            }
            std::cout << "\n";
        }
    }
    std::cout << "\n";

    // Show how BOM references are used for dependency tracking
    std::cout << "BOM Reference Usage:\n";
    std::cout << "  - Each component has a unique BOM reference (bomRef field)\n";
    std::cout << "  - Dependencies are stored as BOM references, not component names\n";
    std::cout << "  - This allows for precise dependency tracking even with version changes\n";
    std::cout << "  - CycloneDX parsers can extract and validate these dependencies\n";
    std::cout << "  - SPDX parsers use SPDXID as the BOM reference\n\n";

    return 0;
} 