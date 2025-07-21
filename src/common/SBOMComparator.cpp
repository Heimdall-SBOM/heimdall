#include "SBOMComparator.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <set>
#include <map>
#include <regex>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include "../compat/compatibility.hpp"

namespace heimdall {

// SBOMComponent implementation

std::string SBOMComponent::getHash() const {
    // Simple hash based on key component fields
    std::string hash = name + ":" + version + ":" + type + ":" + purl;
    return hash;
}

bool SBOMComponent::operator==(const SBOMComponent& other) const {
    return getHash() == other.getHash();
}

bool SBOMComponent::operator!=(const SBOMComponent& other) const {
    return !(*this == other);
}

// SPDX Parser Implementation

std::vector<SBOMComponent> SPDXParser::parse(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return {};
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    return parseContent(content);
}

std::vector<SBOMComponent> SPDXParser::parseContent(const std::string& content) {
    // Detect SPDX version and format
    if (content.find("SPDXVersion:") != std::string::npos) {
        return parseSPDX2_3(content);
    } else if (content.find("\"spdxVersion\"") != std::string::npos) {
        return parseSPDX3_0(content);
    }
    
    return {};
}

bool SPDXParser::processSPDXLine(const std::string& line, SBOMComponent& component) {
    if (line.find("PackageVersion:") != std::string::npos) {
        component.version = line.substr(line.find(":") + 1);
        component.version.erase(0, component.version.find_first_not_of(" \t"));
        component.version.erase(component.version.find_last_not_of(" \t") + 1);
        return false;
    } else if (line.find("PackageSPDXID:") != std::string::npos) {
        component.id = line.substr(line.find(":") + 1);
        component.id.erase(0, component.id.find_first_not_of(" \t"));
        component.id.erase(component.id.find_last_not_of(" \t") + 1);
        return false;
    } else if (line.find("PackageLicenseConcluded:") != std::string::npos) {
        component.license = line.substr(line.find(":") + 1);
        component.license.erase(0, component.license.find_first_not_of(" \t"));
        component.license.erase(component.license.find_last_not_of(" \t") + 1);
        return false;
    } else if (line.find("PackageDownloadLocation:") != std::string::npos) {
        component.purl = line.substr(line.find(":") + 1);
        component.purl.erase(0, component.purl.find_first_not_of(" \t"));
        component.purl.erase(component.purl.find_last_not_of(" \t") + 1);
        return false;
    } else if (line.find("PackageName:") != std::string::npos) {
        // Next package starts
        return true;
    }
    return false;
}

std::vector<SBOMComponent> SPDXParser::parseSPDX2_3(const std::string& content) {
    std::vector<SBOMComponent> components;
    std::istringstream iss(content);
    std::string line;
    
    while (std::getline(iss, line)) {
        if (line.find("PackageName:") != std::string::npos) {
            SBOMComponent component;
            component.name = line.substr(line.find(":") + 1);
            // Trim whitespace
            component.name.erase(0, component.name.find_first_not_of(" \t"));
            component.name.erase(component.name.find_last_not_of(" \t") + 1);
            
            // Look for version, id, and other fields in next few lines
            std::streampos pos = iss.tellg();
            for (int i = 0; i < 10; ++i) {
                std::string nextLine;
                if (!std::getline(iss, nextLine)) break;
                
                bool shouldBreak = processSPDXLine(nextLine, component);
                if (shouldBreak) {
                    iss.seekg(pos);
                    break;
                }
                
                pos = iss.tellg();
            }
            iss.seekg(pos);
            component.type = "package";
            components.push_back(component);
        }
    }
    
    return components;
}

std::vector<SBOMComponent> SPDXParser::parseSPDX3_0(const std::string& content) {
    std::vector<SBOMComponent> components;
    
    // Basic regex-based extraction for demonstration
    std::regex packageRegex("\"name\"\\s*:\\s*\"([^\"]+)\"[^}]*\"versionInfo\"\\s*:\\s*\"([^\"]*)\"[^}]*\"SPDXID\"\\s*:\\s*\"([^\"]+)\"");
    std::sregex_iterator iter(content.begin(), content.end(), packageRegex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        SBOMComponent component;
        component.name = (*iter)[1];
        component.version = (*iter)[2];
        component.id = (*iter)[3];
        component.type = "package";
        components.push_back(component);
    }
    
    return components;
}

// CycloneDX Parser Implementation

std::vector<SBOMComponent> CycloneDXParser::parse(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return {};
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    return parseContent(content);
}

std::string CycloneDXParser::extractVersion(const std::string& content) const {
    size_t pos = content.find("\"specVersion\"");
    if (pos == std::string::npos) {
        return "";
    }
    
    size_t start = content.find("\"", pos + 13) + 1;
    if (start == std::string::npos) {
        return "";
    }
    
    size_t end = content.find("\"", start);
    if (end == std::string::npos) {
        return "";
    }
    
    return content.substr(start, end - start);
}

std::vector<SBOMComponent> CycloneDXParser::parseContent(const std::string& content) {
    // Detect CycloneDX version
    if (content.find("\"specVersion\"") == std::string::npos) {
        return {};
    }
    
    std::string version = extractVersion(content);
    if (version.empty()) {
        return {};
    }
    
    if (version == "1.4") {
        return parseCycloneDX1_4(content);
    } else if (version == "1.5") {
        return parseCycloneDX1_5(content);
    } else if (version == "1.6") {
        return parseCycloneDX1_6(content);
    }
    
    return {};
}

std::vector<SBOMComponent> CycloneDXParser::parseCycloneDX1_4(const std::string& content) const {
    return parseCycloneDX1_6(content); // Use 1.6 parser for now
}

std::vector<SBOMComponent> CycloneDXParser::parseCycloneDX1_5(const std::string& content) const {
    return parseCycloneDX1_6(content); // Use 1.6 parser for now
}

std::vector<SBOMComponent> CycloneDXParser::parseCycloneDX1_6(const std::string& content) const {
    std::vector<SBOMComponent> components;
    
    // Basic regex-based extraction for demonstration
    std::regex componentRegex("\"name\"\\s*:\\s*\"([^\"]+)\"[^}]*\"version\"\\s*:\\s*\"([^\"]*)\"[^}]*\"bom-ref\"\\s*:\\s*\"([^\"]+)\"");
    std::sregex_iterator iter(content.begin(), content.end(), componentRegex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        SBOMComponent component;
        component.name = (*iter)[1];
        component.version = (*iter)[2];
        component.id = (*iter)[3];
        component.type = "library";
        components.push_back(component);
    }
    
    return components;
}

// SBOM Comparator Implementation

std::string SBOMComparator::detectFormatFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return "";
    }
    
    std::string firstLine;
    std::getline(file, firstLine);
    if (firstLine.find("SPDXVersion:") != std::string::npos) {
        return "spdx";
    } else if (firstLine.find("{") != std::string::npos) {
        std::string content;
        file.seekg(0);
        std::stringstream buffer;
        buffer << file.rdbuf();
        content = buffer.str();
        
        if (content.find("\"spdxVersion\"") != std::string::npos) {
            return "spdx";
        } else if (content.find("\"bomFormat\"") != std::string::npos) {
            return "cyclonedx";
        }
    }
    
    return "";
}

std::vector<SBOMDifference> SBOMComparator::compare(const std::string& oldSBOM, const std::string& newSBOM) {
    // Auto-detect format
    std::string format = detectFormatFromFile(oldSBOM);
    
    if (format.empty()) {
        return {};
    }
    
    auto parser = createParser(format);
    if (!parser) {
        return {};
    }
    
    auto oldComponents = parser->parse(oldSBOM);
    auto newComponents = parser->parse(newSBOM);
    
    return compareComponents(oldComponents, newComponents);
}

std::vector<SBOMDifference> SBOMComparator::compareContent(const std::string& oldContent, 
                                                          const std::string& newContent, 
                                                          const std::string& format) {
    auto parser = createParser(format);
    if (!parser) {
        return {};
    }
    
    auto oldComponents = parser->parseContent(oldContent);
    auto newComponents = parser->parseContent(newContent);
    
    return compareComponents(oldComponents, newComponents);
}

std::string SBOMComparator::merge(const std::vector<std::string>& sbomFiles, 
                                 const std::string& outputFormat, 
                                 const std::string& outputVersion) {
    std::vector<std::vector<SBOMComponent>> componentLists;
    
    for (const auto& file : sbomFiles) {
        std::string format = detectFormatFromFile(file);
        
        if (!format.empty()) {
            auto parser = createParser(format);
            if (parser) {
                componentLists.push_back(parser->parse(file));
            }
        }
    }
    
    return mergeComponents(componentLists, outputFormat, outputVersion);
}

std::string SBOMComparator::generateDiffReport(const std::vector<SBOMDifference>& differences, 
                                              const std::string& format) {
    if (format == "json") {
        return generateJSONReport(differences);
    } else if (format == "csv") {
        return generateCSVReport(differences);
    } else {
        return generateTextReport(differences);
    }
}

std::map<std::string, int> SBOMComparator::getDiffStatistics(const std::vector<SBOMDifference>& differences) {
    std::map<std::string, int> stats;
    stats["added"] = 0;
    stats["removed"] = 0;
    stats["modified"] = 0;
    stats["unchanged"] = 0;
    
    for (const auto& diff : differences) {
        switch (diff.type) {
            case SBOMDifference::Type::ADDED:
                stats["added"]++;
                break;
            case SBOMDifference::Type::REMOVED:
                stats["removed"]++;
                break;
            case SBOMDifference::Type::MODIFIED:
                stats["modified"]++;
                break;
            case SBOMDifference::Type::UNCHANGED:
                stats["unchanged"]++;
                break;
        }
    }
    
    return stats;
}

// Private helper methods

std::unique_ptr<SBOMParser> SBOMComparator::createParser(const std::string& format) {
    if (format == "spdx" || format == "spdx-2.3" || format == "spdx-3.0" || format == "spdx-3.0.0" || format == "spdx-3.0.1") {
        return heimdall::compat::make_unique<SPDXParser>();
    } else if (format == "cyclonedx" || format == "cyclonedx-1.4" || format == "cyclonedx-1.5" || format == "cyclonedx-1.6") {
        return heimdall::compat::make_unique<CycloneDXParser>();
    } else {
        return nullptr;
    }
}

std::vector<SBOMDifference> SBOMComparator::compareComponents(const std::vector<SBOMComponent>& oldComponents,
                                                             const std::vector<SBOMComponent>& newComponents) {
    std::vector<SBOMDifference> differences;
    std::map<std::string, SBOMComponent> oldMap, newMap;
    
    // Create maps for efficient lookup
    for (const auto& comp : oldComponents) {
        oldMap[comp.id] = comp;
    }
    for (const auto& comp : newComponents) {
        newMap[comp.id] = comp;
    }
    
    // Find added components
    for (const auto& comp : newComponents) {
        if (oldMap.find(comp.id) == oldMap.end()) {
            SBOMDifference diff;
            diff.type = SBOMDifference::Type::ADDED;
            diff.component = comp;
            differences.push_back(diff);
        }
    }
    
    // Find removed components
    for (const auto& comp : oldComponents) {
        if (newMap.find(comp.id) == newMap.end()) {
            SBOMDifference diff;
            diff.type = SBOMDifference::Type::REMOVED;
            diff.component = comp;
            differences.push_back(diff);
        }
    }
    
    // Find modified components
    for (const auto& comp : newComponents) {
        auto it = oldMap.find(comp.id);
        if (it != oldMap.end()) {
            const auto& oldComp = it->second;
            if (oldComp.name != comp.name || oldComp.version != comp.version || oldComp.type != comp.type) {
                SBOMDifference diff;
                diff.type = SBOMDifference::Type::MODIFIED;
                diff.component = comp;
                diff.oldComponent = oldComp;
                differences.push_back(diff);
            } else {
                SBOMDifference diff;
                diff.type = SBOMDifference::Type::UNCHANGED;
                diff.component = comp;
                differences.push_back(diff);
            }
        }
    }
    
    return differences;
}

std::string SBOMComparator::mergeComponents(const std::vector<std::vector<SBOMComponent>>& componentLists,
                                           const std::string& outputFormat,
                                           const std::string& outputVersion) {
    std::map<std::string, SBOMComponent> mergedComponents;
    
    // Merge all components, keeping the latest version of each
    for (const auto& componentList : componentLists) {
        for (const auto& component : componentList) {
            auto it = mergedComponents.find(component.id);
            if (it == mergedComponents.end() || it->second.version < component.version) {
                mergedComponents[component.id] = component;
            }
        }
    }
    
    // Convert to vector
    std::vector<SBOMComponent> result;
    for (const auto& pair : mergedComponents) {
        result.push_back(pair.second);
    }
    
    // Generate output in requested format
    if (outputFormat == "spdx") {
        return generateSPDXOutput(result, outputVersion);
    } else if (outputFormat == "cyclonedx") {
        return generateCycloneDXOutput(result, outputVersion);
    }
    
    return "";
}

std::string SBOMComparator::generateSPDXOutput(const std::vector<SBOMComponent>& components, 
                                              const std::string& version) {
    std::stringstream ss;
    
    if (version == "2.3") {
        ss << "SPDXVersion: SPDX-2.3\n";
        ss << "DataLicense: CC0-1.0\n";
        ss << "SPDXID: SPDXRef-DOCUMENT\n";
        ss << "DocumentName: Merged SBOM\n";
        ss << "DocumentNamespace: https://spdx.org/spdxdocs/merged-sbom\n";
        ss << "Creator: Organization: Heimdall SBOM Generator\n";
        ss << "Created: " << getCurrentTimestamp() << "\n\n";
        
        for (const auto& comp : components) {
            ss << "PackageName: " << comp.name << "\n";
            ss << "SPDXID: " << comp.id << "\n";
            if (!comp.version.empty()) {
                ss << "PackageVersion: " << comp.version << "\n";
            }
            ss << "PackageSupplier: NOASSERTION\n";
            ss << "PackageDownloadLocation: NOASSERTION\n";
            ss << "FilesAnalyzed: false\n";
            ss << "PackageLicenseConcluded: NOASSERTION\n";
            ss << "PackageLicenseDeclared: NOASSERTION\n";
            ss << "PackageCopyrightText: NOASSERTION\n\n";
        }
    } else {
        // SPDX 3.0 JSON format
        ss << "{\n";
        ss << "  \"spdxVersion\": \"SPDX-3.0\",\n";
        ss << "  \"creationInfo\": {\n";
        ss << "    \"creators\": [\"Organization: Heimdall SBOM Generator\"],\n";
        ss << "    \"created\": \"" << getCurrentTimestamp() << "\"\n";
        ss << "  },\n";
        ss << "  \"packages\": [\n";
        
        for (size_t i = 0; i < components.size(); ++i) {
            const auto& comp = components[i];
            ss << "    {\n";
            ss << "      \"SPDXID\": \"" << comp.id << "\",\n";
            ss << "      \"name\": \"" << comp.name << "\"";
            if (!comp.version.empty()) {
                ss << ",\n      \"versionInfo\": \"" << comp.version << "\"";
            }
            ss << "\n    }";
            if (i < components.size() - 1) {
                ss << ",";
            }
            ss << "\n";
        }
        
        ss << "  ]\n";
        ss << "}\n";
    }
    
    return ss.str();
}

std::string SBOMComparator::generateCycloneDXOutput(const std::vector<SBOMComponent>& components, 
                                                   const std::string& version) {
    std::stringstream ss;
    
    ss << "{\n";
    ss << "  \"bomFormat\": \"CycloneDX\",\n";
    ss << "  \"specVersion\": \"" << version << "\",\n";
    ss << "  \"metadata\": {\n";
    ss << "    \"timestamp\": \"" << getCurrentTimestamp() << "\",\n";
    ss << "    \"tools\": [{\n";
    ss << "      \"vendor\": \"Heimdall\",\n";
    ss << "      \"name\": \"SBOM Generator\",\n";
    ss << "      \"version\": \"1.0.0\"\n";
    ss << "    }]\n";
    ss << "  },\n";
    ss << "  \"components\": [\n";
    
    for (size_t i = 0; i < components.size(); ++i) {
        const auto& comp = components[i];
        ss << "    {\n";
        ss << "      \"bom-ref\": \"" << comp.id << "\",\n";
        ss << "      \"type\": \"library\",\n";
        ss << "      \"name\": \"" << comp.name << "\"";
        if (!comp.version.empty()) {
            ss << ",\n      \"version\": \"" << comp.version << "\"";
        }
        ss << "\n    }";
        if (i < components.size() - 1) {
            ss << ",";
        }
        ss << "\n";
    }
    
    ss << "  ]\n";
    ss << "}\n";
    
    return ss.str();
}

std::string SBOMComparator::generateJSONReport(const std::vector<SBOMDifference>& differences) {
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"differences\": [\n";
    
    for (size_t i = 0; i < differences.size(); ++i) {
        const auto& diff = differences[i];
        ss << "    {\n";
        ss << "      \"type\": \"" << getDifferenceTypeString(diff.type) << "\",\n";
        ss << "      \"component\": {\n";
        ss << "        \"id\": \"" << diff.component.id << "\",\n";
        ss << "        \"name\": \"" << diff.component.name << "\",\n";
        ss << "        \"version\": \"" << diff.component.version << "\",\n";
        ss << "        \"type\": \"" << diff.component.type << "\"\n";
        ss << "      }";
        
        if (diff.type == SBOMDifference::Type::MODIFIED && diff.oldComponent.has_value()) {
            ss << ",\n      \"oldComponent\": {\n";
            ss << "        \"id\": \"" << diff.oldComponent->id << "\",\n";
            ss << "        \"name\": \"" << diff.oldComponent->name << "\",\n";
            ss << "        \"version\": \"" << diff.oldComponent->version << "\",\n";
            ss << "        \"type\": \"" << diff.oldComponent->type << "\"\n";
            ss << "      }";
        }
        
        ss << "\n    }";
        if (i < differences.size() - 1) {
            ss << ",";
        }
        ss << "\n";
    }
    
    ss << "  ]\n";
    ss << "}\n";
    
    return ss.str();
}

std::string SBOMComparator::generateCSVReport(const std::vector<SBOMDifference>& differences) {
    std::stringstream ss;
    ss << "Type,ID,Name,Version,Type,OldName,OldVersion,OldType\n";
    
    for (const auto& diff : differences) {
        ss << getDifferenceTypeString(diff.type) << ","
           << diff.component.id << ","
           << diff.component.name << ","
           << diff.component.version << ","
           << diff.component.type;
        
        if (diff.type == SBOMDifference::Type::MODIFIED && diff.oldComponent.has_value()) {
            ss << "," << diff.oldComponent->name << ","
               << diff.oldComponent->version << ","
               << diff.oldComponent->type;
        } else {
            ss << ",,,";
        }
        ss << "\n";
    }
    
    return ss.str();
}

std::string SBOMComparator::generateTextReport(const std::vector<SBOMDifference>& differences) {
    std::stringstream ss;
    ss << "SBOM Comparison Report\n";
    ss << "=====================\n\n";
    
    auto stats = getDiffStatistics(differences);
    ss << "Summary:\n";
    ss << "  Added: " << stats["added"] << "\n";
    ss << "  Removed: " << stats["removed"] << "\n";
    ss << "  Modified: " << stats["modified"] << "\n";
    ss << "  Unchanged: " << stats["unchanged"] << "\n\n";
    
    if (differences.empty()) {
        ss << "No differences found\n";
    } else {
        ss << "Details:\n";
        for (const auto& diff : differences) {
            switch (diff.type) {
                case SBOMDifference::Type::ADDED:
                    ss << "[ADDED] " << diff.component.name << " " << diff.component.version << " (" << diff.component.id << ")\n";
                    break;
                case SBOMDifference::Type::REMOVED:
                    ss << "[REMOVED] " << diff.component.name << " " << diff.component.version << " (" << diff.component.id << ")\n";
                    break;
                case SBOMDifference::Type::MODIFIED:
                    ss << "[MODIFIED] " << diff.component.name << " " << diff.component.version << " (" << diff.component.id << ")\n";
                    if (diff.oldComponent.has_value()) {
                        ss << "  Previous: " << diff.oldComponent->name << " " << diff.oldComponent->version << "\n";
                    }
                    break;
                case SBOMDifference::Type::UNCHANGED:
                    ss << "[UNCHANGED] " << diff.component.name << " " << diff.component.version << " (" << diff.component.id << ")\n";
                    break;
            }
        }
    }
    
    return ss.str();
}

std::string SBOMComparator::getDifferenceTypeString(SBOMDifference::Type type) {
    switch (type) {
        case SBOMDifference::Type::ADDED:
            return "added";
        case SBOMDifference::Type::REMOVED:
            return "removed";
        case SBOMDifference::Type::MODIFIED:
            return "modified";
        case SBOMDifference::Type::UNCHANGED:
            return "unchanged";
        default:
            return "unknown";
    }
}

std::string SBOMComparator::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
#if defined(_POSIX_VERSION)
    struct tm tm_buf;
    gmtime_r(&time_t, &tm_buf);
    ss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%SZ");
#else
    // Fallback: not thread-safe
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
#endif
    return ss.str();
}

std::unique_ptr<SBOMParser> SBOMParserFactory::createParser(const std::string& format) {
    std::string lowerFormat = format;
    std::transform(lowerFormat.begin(), lowerFormat.end(), lowerFormat.begin(), ::tolower);

    if (lowerFormat == "spdx" || lowerFormat == "spdx-2.3" || lowerFormat == "spdx-3.0" || lowerFormat == "spdx-3.0.0" || lowerFormat == "spdx-3.0.1") {
        return heimdall::compat::make_unique<SPDXParser>();
    } else if (lowerFormat == "cyclonedx" || lowerFormat == "cyclonedx-1.4" || lowerFormat == "cyclonedx-1.5" || lowerFormat == "cyclonedx-1.6") {
        return heimdall::compat::make_unique<CycloneDXParser>();
    } else {
        return nullptr;
    }
}

std::vector<std::string> SBOMParserFactory::getSupportedFormats() {
    return {"spdx", "cyclonedx"};
}

} // namespace heimdall 
