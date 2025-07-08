#include <gtest/gtest.h>
#include <algorithm>
#include "../src/common/SBOMValidator.hpp"
#include "../src/common/SBOMComparator.hpp"

using namespace heimdall;

class SBOMValidationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test SPDX 2.3 content
        spdx2_3_content = R"(
SPDXVersion: SPDX-2.3
DataLicense: CC0-1.0
SPDXID: SPDXRef-DOCUMENT
DocumentName: Test Document
DocumentNamespace: https://spdx.org/spdxdocs/test
Creator: Organization: Test Org
Created: 2024-01-01T00:00:00Z

PackageName: test-package
PackageVersion: 1.0.0
PackageSPDXID: SPDXRef-Package-test
PackageLicenseConcluded: MIT
PackageDownloadLocation: https://example.com/test
)";

        // Create test CycloneDX 1.6 content
        cyclonedx_content = R"({
  "bomFormat": "CycloneDX",
  "specVersion": "1.6",
  "version": 1,
  "metadata": {
    "timestamp": "2024-01-01T00:00:00Z",
    "tools": [{
      "vendor": "Test",
      "name": "Test Tool",
      "version": "1.0.0"
    }]
  },
  "components": [{
    "type": "library",
    "name": "test-lib",
    "version": "1.0.0",
    "bom-ref": "test-lib-1.0.0"
  }]
})";
    }

    std::string spdx2_3_content;
    std::string cyclonedx_content;
};

TEST_F(SBOMValidationTest, SPDXValidatorCreation) {
    auto validator = SBOMValidatorFactory::createValidator("spdx");
    ASSERT_NE(validator, nullptr);
    EXPECT_EQ(validator->getName(), "SPDX Validator");
}

TEST_F(SBOMValidationTest, CycloneDXValidatorCreation) {
    auto validator = SBOMValidatorFactory::createValidator("cyclonedx");
    ASSERT_NE(validator, nullptr);
    EXPECT_EQ(validator->getName(), "CycloneDX Validator");
}

TEST_F(SBOMValidationTest, InvalidValidatorCreation) {
    auto validator = SBOMValidatorFactory::createValidator("invalid");
    EXPECT_EQ(validator, nullptr);
}

TEST_F(SBOMValidationTest, SPDX2_3Validation) {
    auto validator = SBOMValidatorFactory::createValidator("spdx");
    ASSERT_NE(validator, nullptr);
    
    auto result = validator->validateContent(spdx2_3_content);
    EXPECT_TRUE(result.isValid);
    EXPECT_EQ(result.metadata["format"], "SPDX 2.3");
    EXPECT_EQ(result.metadata["version"], "2.3");
}

TEST_F(SBOMValidationTest, CycloneDX1_6Validation) {
    auto validator = SBOMValidatorFactory::createValidator("cyclonedx");
    ASSERT_NE(validator, nullptr);
    
    auto result = validator->validateContent(cyclonedx_content);
    EXPECT_TRUE(result.isValid);
    EXPECT_EQ(result.metadata["format"], "CycloneDX");
    EXPECT_EQ(result.metadata["version"], "1.6");
}

TEST_F(SBOMValidationTest, SPDXValidationWithErrors) {
    auto validator = SBOMValidatorFactory::createValidator("spdx");
    ASSERT_NE(validator, nullptr);
    
    std::string invalid_content = "Invalid SPDX content";
    auto result = validator->validateContent(invalid_content);
    EXPECT_FALSE(result.isValid);
    EXPECT_FALSE(result.errors.empty());
}

TEST_F(SBOMValidationTest, CycloneDXValidationWithErrors) {
    auto validator = SBOMValidatorFactory::createValidator("cyclonedx");
    ASSERT_NE(validator, nullptr);
    
    std::string invalid_content = "Invalid CycloneDX content";
    auto result = validator->validateContent(invalid_content);
    EXPECT_FALSE(result.isValid);
    EXPECT_FALSE(result.errors.empty());
}

TEST_F(SBOMValidationTest, SupportedFormats) {
    auto formats = SBOMValidatorFactory::getSupportedFormats();
    EXPECT_EQ(formats.size(), 2);
    EXPECT_NE(std::find(formats.begin(), formats.end(), "spdx"), formats.end());
    EXPECT_NE(std::find(formats.begin(), formats.end(), "cyclonedx"), formats.end());
}

// SBOM Comparison Tests

TEST_F(SBOMValidationTest, SBOMComponentEquality) {
    SBOMComponent comp1{"id1", "name1", "1.0.0", "library", "purl1", "MIT", {}, {}};
    SBOMComponent comp2{"id1", "name1", "1.0.0", "library", "purl1", "MIT", {}, {}};
    SBOMComponent comp3{"id2", "name2", "2.0.0", "library", "purl2", "MIT", {}, {}};
    
    EXPECT_EQ(comp1, comp2);
    EXPECT_NE(comp1, comp3);
}

TEST_F(SBOMValidationTest, SBOMComponentHash) {
    SBOMComponent comp1{"id1", "name1", "1.0.0", "library", "purl1", "MIT", {}, {}};
    SBOMComponent comp2{"id1", "name1", "1.0.0", "library", "purl1", "MIT", {}, {}};
    
    EXPECT_EQ(comp1.getHash(), comp2.getHash());
}

TEST_F(SBOMValidationTest, SPDXParserCreation) {
    auto parser = SBOMParserFactory::createParser("spdx");
    ASSERT_NE(parser, nullptr);
    EXPECT_EQ(parser->getName(), "SPDX Parser");
}

TEST_F(SBOMValidationTest, CycloneDXParserCreation) {
    auto parser = SBOMParserFactory::createParser("cyclonedx");
    ASSERT_NE(parser, nullptr);
    EXPECT_EQ(parser->getName(), "CycloneDX Parser");
}

TEST_F(SBOMValidationTest, SPDXParserExtraction) {
    auto parser = SBOMParserFactory::createParser("spdx");
    ASSERT_NE(parser, nullptr);
    
    auto components = parser->parseContent(spdx2_3_content);
    EXPECT_EQ(components.size(), 1);
    EXPECT_EQ(components[0].name, "test-package");
    EXPECT_EQ(components[0].version, "1.0.0");
    EXPECT_EQ(components[0].id, "SPDXRef-Package-test");
}

TEST_F(SBOMValidationTest, CycloneDXParserExtraction) {
    auto parser = SBOMParserFactory::createParser("cyclonedx");
    ASSERT_NE(parser, nullptr);
    
    auto components = parser->parseContent(cyclonedx_content);
    EXPECT_EQ(components.size(), 1);
    EXPECT_EQ(components[0].name, "test-lib");
    EXPECT_EQ(components[0].version, "1.0.0");
    EXPECT_EQ(components[0].id, "test-lib-1.0.0");
}

TEST_F(SBOMValidationTest, SBOMComparatorCreation) {
    SBOMComparator comparator;
    // Just test that it can be created without errors
    EXPECT_TRUE(true);
}

TEST_F(SBOMValidationTest, DiffStatistics) {
    SBOMComparator comparator;
    std::vector<SBOMDifference> differences;
    
    SBOMComponent comp1{"comp1", "Component1", "1.0.0", "library", "", "", {}, {}};
    SBOMComponent comp2{"comp2", "Component2", "1.0.0", "library", "", "", {}, {}};
    SBOMComponent comp3_old{"comp3", "Component3", "1.0.0", "library", "", "", {}, {}};
    SBOMComponent comp3_new{"comp3", "Component3", "2.0.0", "library", "", "", {}, {}};
    
    differences.emplace_back(SBOMDifference::Type::ADDED, comp1);
    differences.emplace_back(SBOMDifference::Type::REMOVED, comp2);
    differences.emplace_back(SBOMDifference::Type::MODIFIED, comp3_new, comp3_old);
    
    auto stats = comparator.getDiffStatistics(differences);
    EXPECT_EQ(stats["added"], 1);
    EXPECT_EQ(stats["removed"], 1);
    EXPECT_EQ(stats["modified"], 1);
    EXPECT_EQ(stats["unchanged"], 0);
}

TEST_F(SBOMValidationTest, DiffReportGeneration) {
    SBOMComparator comparator;
    std::vector<SBOMDifference> differences;
    
    SBOMComponent comp1{"comp1", "Component1", "1.0.0", "library", "", "", {}, {}};
    SBOMComponent comp2{"comp2", "Component2", "1.0.0", "library", "", "", {}, {}};
    SBOMComponent comp3_old{"comp3", "Component3", "1.0.0", "library", "", "", {}, {}};
    SBOMComponent comp3_new{"comp3", "Component3", "2.0.0", "library", "", "", {}, {}};
    
    differences.emplace_back(SBOMDifference::Type::ADDED, comp1);
    differences.emplace_back(SBOMDifference::Type::REMOVED, comp2);
    differences.emplace_back(SBOMDifference::Type::MODIFIED, comp3_new, comp3_old);
    
    auto textReport = comparator.generateDiffReport(differences, "text");
    EXPECT_FALSE(textReport.empty());
    EXPECT_NE(textReport.find("Added:"), std::string::npos);
    
    auto jsonReport = comparator.generateDiffReport(differences, "json");
    EXPECT_FALSE(jsonReport.empty());
    EXPECT_NE(jsonReport.find("\"type\": \"added\""), std::string::npos);
    
    auto csvReport = comparator.generateDiffReport(differences, "csv");
    EXPECT_FALSE(csvReport.empty());
    EXPECT_NE(csvReport.find("added"), std::string::npos);
} 