#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <optional>

namespace heimdall {

/**
 * @brief Represents a component in an SBOM
 */
struct SBOMComponent {
    std::string id;
    std::string name;
    std::string version;
    std::string type;
    std::string purl;
    std::string license;
    std::map<std::string, std::string> properties;
    std::vector<std::string> dependencies;
    
    // Hash for comparison
    std::string getHash() const;
    
    // Compare with another component
    bool operator==(const SBOMComponent& other) const;
    bool operator!=(const SBOMComponent& other) const;
};

/**
 * @brief Represents a difference between two SBOMs
 */
struct SBOMDifference {
    enum class Type {
        ADDED,      // Component added in new SBOM
        REMOVED,    // Component removed from old SBOM
        MODIFIED,   // Component modified between SBOMs
        UNCHANGED   // Component unchanged
    };
    
    Type type;
    SBOMComponent component;
    std::optional<SBOMComponent> oldComponent;
    
    SBOMDifference() = default;
    SBOMDifference(Type t, const SBOMComponent& comp) : type(t), component(comp) {}
    SBOMDifference(Type t, const SBOMComponent& comp, const SBOMComponent& oldComp) 
        : type(t), component(comp), oldComponent(oldComp) {}
};

/**
 * @brief Abstract base class for SBOM parsers
 */
class SBOMParser {
public:
    virtual ~SBOMParser() = default;
    
    /**
     * @brief Parse SBOM file and extract components
     * @param filePath Path to SBOM file
     * @return Vector of components
     */
    virtual std::vector<SBOMComponent> parse(const std::string& filePath) = 0;
    
    /**
     * @brief Parse SBOM content from string
     * @param content SBOM content
     * @return Vector of components
     */
    virtual std::vector<SBOMComponent> parseContent(const std::string& content) = 0;
    
    /**
     * @brief Get parser name
     * @return Parser name
     */
    virtual std::string getName() const = 0;
};

/**
 * @brief SPDX parser implementation
 */
class SPDXParser : public SBOMParser {
public:
    std::vector<SBOMComponent> parse(const std::string& filePath) override;
    std::vector<SBOMComponent> parseContent(const std::string& content) override;
    std::string getName() const override { return "SPDX Parser"; }
    
private:
    bool processSPDXLine(const std::string& line, SBOMComponent& component);
    std::vector<SBOMComponent> parseSPDX2_3(const std::string& content);
    std::vector<SBOMComponent> parseSPDX3_0(const std::string& content);
};

/**
 * @brief CycloneDX parser implementation
 */
class CycloneDXParser : public SBOMParser {
public:
    std::vector<SBOMComponent> parse(const std::string& filePath) override;
    std::vector<SBOMComponent> parseContent(const std::string& content) override;
    std::string getName() const override { return "CycloneDX Parser"; }
    
private:
    std::string extractVersion(const std::string& content) const;
    std::vector<SBOMComponent> parseCycloneDX1_4(const std::string& content) const;
    std::vector<SBOMComponent> parseCycloneDX1_5(const std::string& content) const;
    std::vector<SBOMComponent> parseCycloneDX1_6(const std::string& content) const;
};

/**
 * @brief SBOM comparison and merging functionality
 */
class SBOMComparator {
public:
    /**
     * @brief Compare two SBOM files
     * @param oldSBOM Path to old SBOM file
     * @param newSBOM Path to new SBOM file
     * @return Vector of differences
     */
    std::vector<SBOMDifference> compare(const std::string& oldSBOM, const std::string& newSBOM);
    
    /**
     * @brief Compare two SBOM contents
     * @param oldContent Old SBOM content
     * @param newContent New SBOM content
     * @param format SBOM format ("spdx" or "cyclonedx")
     * @return Vector of differences
     */
    std::vector<SBOMDifference> compareContent(const std::string& oldContent, 
                                              const std::string& newContent, 
                                              const std::string& format);
    
    /**
     * @brief Merge multiple SBOMs into one
     * @param sbomFiles Vector of SBOM file paths
     * @param outputFormat Output format ("spdx" or "cyclonedx")
     * @param outputVersion Output version
     * @return Merged SBOM content
     */
    std::string merge(const std::vector<std::string>& sbomFiles, 
                     const std::string& outputFormat, 
                     const std::string& outputVersion);
    
    /**
     * @brief Generate diff report
     * @param differences Vector of differences
     * @param format Output format ("text", "json", "csv")
     * @return Diff report
     */
    std::string generateDiffReport(const std::vector<SBOMDifference>& differences, 
                                  const std::string& format = "text");
    
    /**
     * @brief Get statistics about differences
     * @param differences Vector of differences
     * @return Map of statistics
     */
    std::map<std::string, int> getDiffStatistics(const std::vector<SBOMDifference>& differences);
    
private:
    std::string detectFormatFromFile(const std::string& filePath);
    std::unique_ptr<SBOMParser> createParser(const std::string& format);
    std::vector<SBOMDifference> compareComponents(const std::vector<SBOMComponent>& oldComponents,
                                                 const std::vector<SBOMComponent>& newComponents);
    std::string mergeComponents(const std::vector<std::vector<SBOMComponent>>& componentLists,
                               const std::string& outputFormat, const std::string& outputVersion);
    std::string generateSPDXOutput(const std::vector<SBOMComponent>& components, 
                                  const std::string& version);
    std::string generateCycloneDXOutput(const std::vector<SBOMComponent>& components, 
                                       const std::string& version);
    std::string generateJSONReport(const std::vector<SBOMDifference>& differences);
    std::string generateCSVReport(const std::vector<SBOMDifference>& differences);
    std::string generateTextReport(const std::vector<SBOMDifference>& differences);
    std::string getDifferenceTypeString(SBOMDifference::Type type);
    std::string getCurrentTimestamp();
};

/**
 * @brief Factory for creating SBOM parsers
 */
class SBOMParserFactory {
public:
    /**
     * @brief Create parser for given format
     * @param format SBOM format ("spdx" or "cyclonedx")
     * @return Parser instance
     */
    static std::unique_ptr<SBOMParser> createParser(const std::string& format);
    
    /**
     * @brief Get supported formats
     * @return List of supported formats
     */
    static std::vector<std::string> getSupportedFormats();
};

} // namespace heimdall 