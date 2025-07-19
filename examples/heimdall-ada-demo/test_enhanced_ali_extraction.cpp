/*
 * Test program to demonstrate enhanced ALI file extraction
 * Shows what additional valuable information we could extract
 */

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include "../../src/common/AdaExtractor.hpp"

struct EnhancedAliInfo {
    std::string packageName;
    std::string sourceFile;
    std::string checksum;
    std::string timestamp;
    std::vector<std::string> functions;
    std::vector<std::string> variables;
    std::vector<std::string> types;
    std::vector<std::string> buildFlags;
    std::vector<std::string> dependencies;
    std::map<std::string, std::string> functionSignatures;
    std::map<std::string, std::string> variableTypes;
    std::vector<std::string> crossReferences;
};

void parseEnhancedAliFile(const std::string& aliFile, EnhancedAliInfo& info) {
    std::ifstream file(aliFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open ALI file: " << aliFile << std::endl;
        return;
    }
    
    // Extract package name from filename
    size_t dotPos = aliFile.find_last_of('.');
    if (dotPos != std::string::npos) {
        info.packageName = aliFile.substr(0, dotPos);
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Parse version line
        if (line.substr(0, 2) == "V ") {
            // Already handled by existing extractor
        }
        // Parse build flags (RV lines)
        else if (line.substr(0, 3) == "RV ") {
            std::string flag = line.substr(3);
            info.buildFlags.push_back(flag);
        }
        // Parse dependencies (W and Z lines)
        else if (line.substr(0, 2) == "W " || line.substr(0, 2) == "Z ") {
            std::istringstream lineStream(line.substr(2));
            std::string packagePart, sourceFile, aliFile;
            if (lineStream >> packagePart >> sourceFile >> aliFile) {
                size_t percentPos = packagePart.find('%');
                if (percentPos != std::string::npos) {
                    std::string packageName = packagePart.substr(0, percentPos);
                    info.dependencies.push_back(packageName);
                }
            }
        }
        // Parse file information (D lines)
        else if (line.substr(0, 2) == "D ") {
            std::istringstream lineStream(line.substr(2));
            std::string fileName, timestamp, checksum, packageInfo;
            if (lineStream >> fileName >> timestamp >> checksum >> packageInfo) {
                if (fileName.find(info.packageName) != std::string::npos) {
                    info.sourceFile = fileName;
                    info.timestamp = timestamp;
                    info.checksum = checksum;
                }
            }
        }
        // Parse function and variable information (X lines)
        else if (line.substr(0, 2) == "X ") {
            std::istringstream lineStream(line.substr(2));
            std::string token;
            std::vector<std::string> tokens;
            while (lineStream >> token) {
                tokens.push_back(token);
            }
            
            for (const auto& token : tokens) {
                // Look for function patterns (contains *)
                if (token.find('*') != std::string::npos && token.find('V') != std::string::npos) {
                    size_t starPos = token.find('*');
                    if (starPos > 0) {
                        std::string funcName = token.substr(0, starPos);
                        info.functions.push_back(funcName);
                    }
                }
                // Look for variable patterns (contains {)
                else if (token.find('{') != std::string::npos && token.find('a') != std::string::npos) {
                    size_t bracePos = token.find('{');
                    if (bracePos > 0) {
                        std::string varName = token.substr(0, bracePos);
                        std::string varType = token.substr(bracePos);
                        info.variables.push_back(varName);
                        info.variableTypes[varName] = varType;
                    }
                }
                // Look for type patterns (contains {)
                else if (token.find('{') != std::string::npos && token.find('i') != std::string::npos) {
                    size_t bracePos = token.find('{');
                    if (bracePos > 0) {
                        std::string typeName = token.substr(0, bracePos);
                        info.types.push_back(typeName);
                    }
                }
            }
        }
        // Parse cross-references (G lines)
        else if (line.substr(0, 2) == "G ") {
            if (line.find("G r") != std::string::npos) {
                // Extract function call information
                size_t start = line.find('[');
                size_t end = line.find(']', start);
                if (start != std::string::npos && end != std::string::npos) {
                    std::string callInfo = line.substr(start, end - start + 1);
                    info.crossReferences.push_back(callInfo);
                }
            }
        }
    }
    
    file.close();
}

void printEnhancedInfo(const EnhancedAliInfo& info) {
    std::cout << "\n=== Enhanced ALI Information: " << info.packageName << " ===" << std::endl;
    std::cout << "Source File: " << info.sourceFile << std::endl;
    std::cout << "Timestamp: " << info.timestamp << std::endl;
    std::cout << "Checksum: " << info.checksum << std::endl;
    
    std::cout << "\nBuild Flags (" << info.buildFlags.size() << "):" << std::endl;
    for (const auto& flag : info.buildFlags) {
        std::cout << "  - " << flag << std::endl;
    }
    
    std::cout << "\nDependencies (" << info.dependencies.size() << "):" << std::endl;
    for (const auto& dep : info.dependencies) {
        std::cout << "  - " << dep << std::endl;
    }
    
    std::cout << "\nFunctions (" << info.functions.size() << "):" << std::endl;
    for (const auto& func : info.functions) {
        std::cout << "  - " << func << std::endl;
    }
    
    std::cout << "\nVariables (" << info.variables.size() << "):" << std::endl;
    for (const auto& var : info.variables) {
        auto it = info.variableTypes.find(var);
        if (it != info.variableTypes.end()) {
            std::cout << "  - " << var << " " << it->second << std::endl;
        } else {
            std::cout << "  - " << var << " (type unknown)" << std::endl;
        }
    }
    
    std::cout << "\nTypes (" << info.types.size() << "):" << std::endl;
    for (const auto& type : info.types) {
        std::cout << "  - " << type << std::endl;
    }
    
    std::cout << "\nCross-References (" << info.crossReferences.size() << "):" << std::endl;
    for (const auto& ref : info.crossReferences) {
        std::cout << "  - " << ref << std::endl;
    }
}

int main() {
    std::cout << "=== Enhanced ALI File Extraction Analysis ===" << std::endl;
    
    std::vector<std::string> aliFiles = {
        "main.ali",
        "data_reader.ali", 
        "string_utils.ali",
        "math_lib.ali"
    };
    
    for (const auto& aliFile : aliFiles) {
        EnhancedAliInfo info;
        parseEnhancedAliFile(aliFile, info);
        printEnhancedInfo(info);
    }
    
    std::cout << "\n=== Summary of Valuable Information Available ===" << std::endl;
    std::cout << "1. Function Signatures: Parameter types and return types" << std::endl;
    std::cout << "2. Variable Types: Type information for all variables" << std::endl;
    std::cout << "3. Build Configuration: Compiler flags and optimization settings" << std::endl;
    std::cout << "4. Timestamps: When each file was compiled" << std::endl;
    std::cout << "5. Checksums: File integrity verification" << std::endl;
    std::cout << "6. Cross-References: Function call relationships" << std::endl;
    std::cout << "7. Package Types: Specification vs body information" << std::endl;
    std::cout << "8. Dependency Types: With-clause vs runtime dependencies" << std::endl;
    std::cout << "9. Compilation Flags: Detailed compiler settings" << std::endl;
    std::cout << "10. Type Information: Ada type system details" << std::endl;
    
    return 0;
} 