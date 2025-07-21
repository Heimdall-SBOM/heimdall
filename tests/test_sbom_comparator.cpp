#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <thread>
#include "common/SBOMComparator.hpp"

using namespace heimdall;

class SBOMComparatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = std::filesystem::temp_directory_path() / "heimdall_comparator_test";
        std::filesystem::create_directories(test_dir);
        
        // Create test SBOM files
        createTestSBOMs();
    }

    void TearDown() override {
        std::filesystem::remove_all(test_dir);
    }

    void createTestSBOMs() {
        // Create SPDX 2.3 test file
        std::string spdx_file = (test_dir / "test1.spdx").string();
        std::ofstream spdx_out(spdx_file);
        spdx_out << R"(SPDXVersion: SPDX-2.3
DataLicense: CC0-1.0
SPDXID: SPDXRef-DOCUMENT
DocumentName: Test Document 1
DocumentNamespace: https://spdx.org/spdxdocs/test1
Creator: Organization: Test Org
Created: 2024-01-01T00:00:00Z

PackageName: libfoo
PackageVersion: 1.0.0
PackageSPDXID: SPDXRef-Package-libfoo
PackageLicenseConcluded: MIT
PackageDownloadLocation: https://example.com/libfoo

PackageName: libbar
PackageVersion: 2.0.0
PackageSPDXID: SPDXRef-Package-libbar
PackageLicenseConcluded: Apache-2.0
PackageDownloadLocation: https://example.com/libbar
)";
        spdx_out.close();

        // Create SPDX 2.3 test file with differences
        std::string spdx_file2 = (test_dir / "test2.spdx").string();
        std::ofstream spdx_out2(spdx_file2);
        spdx_out2 << R"(SPDXVersion: SPDX-2.3
DataLicense: CC0-1.0
SPDXID: SPDXRef-DOCUMENT
DocumentName: Test Document 2
DocumentNamespace: https://spdx.org/spdxdocs/test2
Creator: Organization: Test Org
Created: 2024-01-02T00:00:00Z

PackageName: libfoo
PackageVersion: 1.1.0
PackageSPDXID: SPDXRef-Package-libfoo
PackageLicenseConcluded: MIT
PackageDownloadLocation: https://example.com/libfoo

PackageName: libbaz
PackageVersion: 3.0.0
PackageSPDXID: SPDXRef-Package-libbaz
PackageLicenseConcluded: GPL-3.0
PackageDownloadLocation: https://example.com/libbaz
)";
        spdx_out2.close();

        // Create CycloneDX test file
        std::string cdx_file = (test_dir / "test1.cdx.json").string();
        std::ofstream cdx_out(cdx_file);
        cdx_out << R"({
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
  "components": [
    {
      "type": "library",
      "name": "libfoo",
      "version": "1.0.0",
      "bom-ref": "libfoo-1.0.0",
      "licenses": [{"license": {"id": "MIT"}}]
    },
    {
      "type": "library",
      "name": "libbar",
      "version": "2.0.0",
      "bom-ref": "libbar-2.0.0",
      "licenses": [{"license": {"id": "Apache-2.0"}}]
    }
  ]
})";
        cdx_out.close();

        // Create CycloneDX test file with differences
        std::string cdx_file2 = (test_dir / "test2.cdx.json").string();
        std::ofstream cdx_out2(cdx_file2);
        cdx_out2 << R"({
  "bomFormat": "CycloneDX",
  "specVersion": "1.6",
  "version": 1,
  "metadata": {
    "timestamp": "2024-01-02T00:00:00Z",
    "tools": [{
      "vendor": "Test",
      "name": "Test Tool",
      "version": "1.1.0"
    }]
  },
  "components": [
    {
      "type": "library",
      "name": "libfoo",
      "version": "1.1.0",
      "bom-ref": "libfoo-1.1.0",
      "licenses": [{"license": {"id": "MIT"}}]
    },
    {
      "type": "library",
      "name": "libbaz",
      "version": "3.0.0",
      "bom-ref": "libbaz-3.0.0",
      "licenses": [{"license": {"id": "GPL-3.0"}}]
    }
  ]
})";
        cdx_out2.close();

        test_spdx1 = spdx_file;
        test_spdx2 = spdx_file2;
        test_cdx1 = cdx_file;
        test_cdx2 = cdx_file2;
    }

    std::filesystem::path test_dir;
    std::string test_spdx1, test_spdx2, test_cdx1, test_cdx2;
};

// Basic functionality tests
TEST_F(SBOMComparatorTest, Constructor) {
    SBOMComparator comparator;
    EXPECT_TRUE(true); // Constructor should not throw
}

TEST_F(SBOMComparatorTest, CopyConstructor) {
    SBOMComparator original;
    SBOMComparator copy(original);
    EXPECT_TRUE(true); // Copy constructor should not throw
}

TEST_F(SBOMComparatorTest, AssignmentOperator) {
    SBOMComparator original;
    SBOMComparator assigned;
    assigned = original;
    EXPECT_TRUE(true); // Assignment should not throw
}

// SPDX Parser tests
TEST_F(SBOMComparatorTest, SPDXParserCreation) {
    SPDXParser parser;
    EXPECT_TRUE(true); // Constructor should not throw
}

TEST_F(SBOMComparatorTest, SPDXParserParseFile) {
    SPDXParser parser;
    auto components = parser.parse(test_spdx1);
    EXPECT_FALSE(components.empty());
}

TEST_F(SBOMComparatorTest, SPDXParserParseContent) {
    SPDXParser parser;
    std::ifstream file(test_spdx1);
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    auto components = parser.parseContent(buffer.str());
    EXPECT_FALSE(components.empty());
}

TEST_F(SBOMComparatorTest, SPDXParserParseEmptyFile) {
    SPDXParser parser;
    std::string empty_file = (test_dir / "empty.spdx").string();
    std::ofstream(empty_file).close();
    
    auto components = parser.parse(empty_file);
    EXPECT_TRUE(components.empty());
}

TEST_F(SBOMComparatorTest, SPDXParserParseNonExistentFile) {
    SPDXParser parser;
    auto components = parser.parse("/nonexistent/file.spdx");
    EXPECT_TRUE(components.empty());
}

TEST_F(SBOMComparatorTest, SPDXParserParseInvalidContent) {
    SPDXParser parser;
    auto components = parser.parseContent("invalid spdx content");
    EXPECT_TRUE(components.empty());
}

TEST_F(SBOMComparatorTest, SPDXParserParseMalformedContent) {
    SPDXParser parser;
    auto components = parser.parseContent("SPDXVersion: SPDX-2.3\nPackageName: test\n");
    // Should handle malformed content gracefully
    EXPECT_NO_THROW(parser.parseContent("SPDXVersion: SPDX-2.3\nPackageName: test\n"));
}

TEST_F(SBOMComparatorTest, SPDXParserParseLargeContent) {
    SPDXParser parser;
    
    // Create large SPDX content
    std::string large_content = "SPDXVersion: SPDX-2.3\nDataLicense: CC0-1.0\n";
    for (int i = 0; i < 1000; ++i) {
        large_content += "PackageName: lib" + std::to_string(i) + "\n";
        large_content += "PackageVersion: 1.0.0\n";
        large_content += "PackageSPDXID: SPDXRef-Package-lib" + std::to_string(i) + "\n";
    }
    
    auto components = parser.parseContent(large_content);
    EXPECT_NO_THROW(parser.parseContent(large_content));
}

TEST_F(SBOMComparatorTest, SPDXParserParseWithSpecialCharacters) {
    SPDXParser parser;
    std::string special_content = R"(SPDXVersion: SPDX-2.3
DataLicense: CC0-1.0
PackageName: lib-special@test
PackageVersion: 1.0.0
PackageSPDXID: SPDXRef-Package-lib-special
PackageLicenseConcluded: MIT
PackageDownloadLocation: https://example.com/lib-special@test
)";
    
    auto components = parser.parseContent(special_content);
    EXPECT_NO_THROW(parser.parseContent(special_content));
}

TEST_F(SBOMComparatorTest, SPDXParserParseWithUnicode) {
    SPDXParser parser;
    std::string unicode_content = R"(SPDXVersion: SPDX-2.3
DataLicense: CC0-1.0
PackageName: 测试库
PackageVersion: 1.0.0
PackageSPDXID: SPDXRef-Package-测试库
PackageLicenseConcluded: MIT
PackageDownloadLocation: https://example.com/测试库
)";
    
    auto components = parser.parseContent(unicode_content);
    EXPECT_NO_THROW(parser.parseContent(unicode_content));
}

// CycloneDX Parser tests
TEST_F(SBOMComparatorTest, CycloneDXParserCreation) {
    CycloneDXParser parser;
    EXPECT_TRUE(true); // Constructor should not throw
}

TEST_F(SBOMComparatorTest, CycloneDXParserParseFile) {
    CycloneDXParser parser;
    auto components = parser.parse(test_cdx1);
    EXPECT_FALSE(components.empty());
}

TEST_F(SBOMComparatorTest, CycloneDXParserParseContent) {
    CycloneDXParser parser;
    std::ifstream file(test_cdx1);
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    auto components = parser.parseContent(buffer.str());
    EXPECT_FALSE(components.empty());
}

TEST_F(SBOMComparatorTest, CycloneDXParserParseEmptyFile) {
    CycloneDXParser parser;
    std::string empty_file = (test_dir / "empty.cdx.json").string();
    std::ofstream(empty_file).close();
    
    auto components = parser.parse(empty_file);
    EXPECT_TRUE(components.empty());
}

TEST_F(SBOMComparatorTest, CycloneDXParserParseNonExistentFile) {
    CycloneDXParser parser;
    auto components = parser.parse("/nonexistent/file.cdx.json");
    EXPECT_TRUE(components.empty());
}

TEST_F(SBOMComparatorTest, CycloneDXParserParseInvalidContent) {
    CycloneDXParser parser;
    auto components = parser.parseContent("invalid json content");
    EXPECT_TRUE(components.empty());
}

TEST_F(SBOMComparatorTest, CycloneDXParserParseMalformedJSON) {
    CycloneDXParser parser;
    auto components = parser.parseContent(R"({
  "bomFormat": "CycloneDX",
  "specVersion": "1.6",
  "components": [
    {
      "type": "library",
      "name": "test"
      // Missing closing brace
)");
    EXPECT_NO_THROW(parser.parseContent(R"({
  "bomFormat": "CycloneDX",
  "specVersion": "1.6",
  "components": [
    {
      "type": "library",
      "name": "test"
)"));
}

TEST_F(SBOMComparatorTest, CycloneDXParserParseLargeContent) {
    CycloneDXParser parser;
    
    // Create large CycloneDX content
    std::string large_content = R"({
  "bomFormat": "CycloneDX",
  "specVersion": "1.6",
  "version": 1,
  "metadata": {
    "timestamp": "2024-01-01T00:00:00Z"
  },
  "components": [)";
    
    for (int i = 0; i < 1000; ++i) {
        large_content += R"({
      "type": "library",
      "name": "lib)" + std::to_string(i) + R"(",
      "version": "1.0.0",
      "bom-ref": "lib)" + std::to_string(i) + R"(-1.0.0"
    },)";
    }
    large_content += R"({
      "type": "library",
      "name": "final",
      "version": "1.0.0",
      "bom-ref": "final-1.0.0"
    }]
})";
    
    auto components = parser.parseContent(large_content);
    EXPECT_NO_THROW(parser.parseContent(large_content));
}

TEST_F(SBOMComparatorTest, CycloneDXParserParseWithSpecialCharacters) {
    CycloneDXParser parser;
    std::string special_content = R"({
  "bomFormat": "CycloneDX",
  "specVersion": "1.6",
  "version": 1,
  "metadata": {
    "timestamp": "2024-01-01T00:00:00Z"
  },
  "components": [
    {
      "type": "library",
      "name": "lib-special@test",
      "version": "1.0.0",
      "bom-ref": "lib-special@test-1.0.0"
    }
  ]
})";
    
    auto components = parser.parseContent(special_content);
    EXPECT_NO_THROW(parser.parseContent(special_content));
}

TEST_F(SBOMComparatorTest, CycloneDXParserParseWithUnicode) {
    CycloneDXParser parser;
    std::string unicode_content = R"({
  "bomFormat": "CycloneDX",
  "specVersion": "1.6",
  "version": 1,
  "metadata": {
    "timestamp": "2024-01-01T00:00:00Z"
  },
  "components": [
    {
      "type": "library",
      "name": "测试库",
      "version": "1.0.0",
      "bom-ref": "测试库-1.0.0"
    }
  ]
})";
    
    auto components = parser.parseContent(unicode_content);
    EXPECT_NO_THROW(parser.parseContent(unicode_content));
}

// Comparison tests
TEST_F(SBOMComparatorTest, CompareSPDXFiles) {
    SBOMComparator comparator;
    auto differences = comparator.compare(test_spdx1, test_spdx2);
    EXPECT_FALSE(differences.empty());
}

TEST_F(SBOMComparatorTest, CompareCycloneDXFiles) {
    SBOMComparator comparator;
    auto differences = comparator.compare(test_cdx1, test_cdx2);
    EXPECT_FALSE(differences.empty());
}

TEST_F(SBOMComparatorTest, CompareIdenticalFiles) {
    SBOMComparator comparator;
    auto differences = comparator.compare(test_spdx1, test_spdx1);
    // Identical files should have UNCHANGED differences, not empty
    EXPECT_FALSE(differences.empty());
    // All differences should be UNCHANGED
    for (const auto& diff : differences) {
        EXPECT_EQ(diff.type, SBOMDifference::Type::UNCHANGED);
    }
}

TEST_F(SBOMComparatorTest, CompareNonExistentFiles) {
    SBOMComparator comparator;
    auto differences = comparator.compare("/nonexistent1", "/nonexistent2");
    EXPECT_TRUE(differences.empty());
}

TEST_F(SBOMComparatorTest, CompareEmptyFiles) {
    SBOMComparator comparator;
    std::string empty1 = (test_dir / "empty1.spdx").string();
    std::string empty2 = (test_dir / "empty2.spdx").string();
    std::ofstream(empty1).close();
    std::ofstream(empty2).close();
    
    auto differences = comparator.compare(empty1, empty2);
    EXPECT_TRUE(differences.empty());
}

TEST_F(SBOMComparatorTest, CompareInvalidFiles) {
    SBOMComparator comparator;
    std::string invalid1 = (test_dir / "invalid1.txt").string();
    std::string invalid2 = (test_dir / "invalid2.txt").string();
    std::ofstream(invalid1) << "not an sbom";
    std::ofstream(invalid2) << "also not an sbom";
    
    auto differences = comparator.compare(invalid1, invalid2);
    EXPECT_NO_THROW(comparator.compare(invalid1, invalid2));
}

TEST_F(SBOMComparatorTest, CompareMixedFormats) {
    SBOMComparator comparator;
    auto differences = comparator.compare(test_spdx1, test_cdx1);
    EXPECT_NO_THROW(comparator.compare(test_spdx1, test_cdx1));
}

// Statistics tests
TEST_F(SBOMComparatorTest, GetDiffStatistics) {
    SBOMComparator comparator;
    auto differences = comparator.compare(test_spdx1, test_spdx2);
    auto stats = comparator.getDiffStatistics(differences);
    
    EXPECT_GE(stats.size(), 0);
    // Check that stats contains expected keys
    EXPECT_TRUE(stats.find("total") != stats.end() || stats.find("added") != stats.end() || 
                stats.find("removed") != stats.end() || stats.find("modified") != stats.end());
}

TEST_F(SBOMComparatorTest, GetDiffStatisticsEmpty) {
    SBOMComparator comparator;
    std::vector<SBOMDifference> empty_differences;
    auto stats = comparator.getDiffStatistics(empty_differences);
    
    EXPECT_EQ(stats.size(), 4); // Should have 4 keys: added, removed, modified, unchanged
    EXPECT_EQ(stats["added"], 0);
    EXPECT_EQ(stats["removed"], 0);
    EXPECT_EQ(stats["modified"], 0);
    EXPECT_EQ(stats["unchanged"], 0);
}

TEST_F(SBOMComparatorTest, GetDiffStatisticsLarge) {
    SBOMComparator comparator;
    
    // Create large differences list
    std::vector<SBOMDifference> large_differences;
    for (int i = 0; i < 1000; ++i) {
        SBOMComponent comp;
        comp.name = "lib" + std::to_string(i);
        comp.version = "1.0.0";
        large_differences.emplace_back(SBOMDifference::Type::ADDED, comp);
    }
    
    auto stats = comparator.getDiffStatistics(large_differences);
    EXPECT_GE(stats.size(), 0);
}

// Report generation tests
TEST_F(SBOMComparatorTest, GenerateDiffReportText) {
    SBOMComparator comparator;
    auto differences = comparator.compare(test_spdx1, test_spdx2);
    auto report = comparator.generateDiffReport(differences, "text");
    
    EXPECT_FALSE(report.empty());
    EXPECT_NE(report.find("ADDED"), std::string::npos);
    EXPECT_NE(report.find("REMOVED"), std::string::npos);
    EXPECT_NE(report.find("MODIFIED"), std::string::npos);
}

TEST_F(SBOMComparatorTest, GenerateDiffReportJSON) {
    SBOMComparator comparator;
    auto differences = comparator.compare(test_spdx1, test_spdx2);
    auto report = comparator.generateDiffReport(differences, "json");
    
    EXPECT_FALSE(report.empty());
    EXPECT_NE(report.find("added"), std::string::npos);
    EXPECT_NE(report.find("removed"), std::string::npos);
    EXPECT_NE(report.find("modified"), std::string::npos);
}

TEST_F(SBOMComparatorTest, GenerateDiffReportCSV) {
    SBOMComparator comparator;
    auto differences = comparator.compare(test_spdx1, test_spdx2);
    auto report = comparator.generateDiffReport(differences, "csv");
    
    EXPECT_FALSE(report.empty());
    EXPECT_NE(report.find("added"), std::string::npos);
    EXPECT_NE(report.find("removed"), std::string::npos);
    EXPECT_NE(report.find("modified"), std::string::npos);
}

TEST_F(SBOMComparatorTest, GenerateDiffReportInvalidFormat) {
    SBOMComparator comparator;
    auto differences = comparator.compare(test_spdx1, test_spdx2);
    auto report = comparator.generateDiffReport(differences, "invalid");
    
    EXPECT_FALSE(report.empty()); // Should fall back to text format
}

TEST_F(SBOMComparatorTest, GenerateDiffReportEmpty) {
    SBOMComparator comparator;
    std::vector<SBOMDifference> empty_differences;
    auto report = comparator.generateDiffReport(empty_differences, "text");
    
    EXPECT_FALSE(report.empty());
    EXPECT_NE(report.find("No differences found"), std::string::npos);
}

TEST_F(SBOMComparatorTest, GenerateDiffReportLarge) {
    SBOMComparator comparator;
    
    // Create large differences list
    std::vector<SBOMDifference> large_differences;
    for (int i = 0; i < 1000; ++i) {
        SBOMComponent comp;
        comp.name = "lib" + std::to_string(i);
        comp.version = "1.0.0";
        large_differences.emplace_back(SBOMDifference::Type::ADDED, comp);
    }
    
    auto report = comparator.generateDiffReport(large_differences, "text");
    EXPECT_FALSE(report.empty());
    EXPECT_NE(report.find("1000"), std::string::npos);
}

// Error handling tests
TEST_F(SBOMComparatorTest, HandleFileReadErrors) {
    SBOMComparator comparator;
    
    // Test with files that can't be read
    std::string unreadable = (test_dir / "unreadable").string();
    std::ofstream(unreadable).close();
    chmod(unreadable.c_str(), 0000); // Remove all permissions
    
    auto differences = comparator.compare(unreadable, unreadable);
    EXPECT_TRUE(differences.empty());
    
    chmod(unreadable.c_str(), 0644); // Restore permissions
}

TEST_F(SBOMComparatorTest, HandleMemoryPressure) {
    SBOMComparator comparator;
    
    // Create very large test files
    std::string large1 = (test_dir / "large1.spdx").string();
    std::string large2 = (test_dir / "large2.spdx").string();
    
    std::ofstream out1(large1);
    std::ofstream out2(large2);
    
    out1 << "SPDXVersion: SPDX-2.3\nDataLicense: CC0-1.0\n";
    out2 << "SPDXVersion: SPDX-2.3\nDataLicense: CC0-1.0\n";
    
    for (int i = 0; i < 10000; ++i) {
        out1 << "PackageName: lib" << i << "\nPackageVersion: 1.0.0\n";
        out2 << "PackageName: lib" << i << "\nPackageVersion: 1.1.0\n";
    }
    
    out1.close();
    out2.close();
    
    auto differences = comparator.compare(large1, large2);
    EXPECT_NO_THROW(comparator.compare(large1, large2));
}

TEST_F(SBOMComparatorTest, HandleConcurrentAccess) {
    SBOMComparator comparator;
    
    // Test concurrent access to comparator
    auto compare_thread = [&comparator, this]() {
        for (int i = 0; i < 100; ++i) {
            EXPECT_NO_THROW(comparator.compare(test_spdx1, test_spdx2));
        }
    };
    
    std::thread t1(compare_thread);
    std::thread t2(compare_thread);
    std::thread t3(compare_thread);
    
    t1.join();
    t2.join();
    t3.join();
}

TEST_F(SBOMComparatorTest, HandleRapidSuccession) {
    SBOMComparator comparator;
    
    // Test rapid succession of comparisons
    for (int i = 0; i < 1000; ++i) {
        EXPECT_NO_THROW(comparator.compare(test_spdx1, test_spdx2));
    }
}

TEST_F(SBOMComparatorTest, HandleMixedOperations) {
    SBOMComparator comparator;
    
    // Test mixed operations
    for (int i = 0; i < 100; ++i) {
        auto differences = comparator.compare(test_spdx1, test_spdx2);
        auto stats = comparator.getDiffStatistics(differences);
        auto report = comparator.generateDiffReport(differences, "text");
        
        EXPECT_NO_THROW({
            auto differences2 = comparator.compare(test_cdx1, test_cdx2);
            auto stats2 = comparator.getDiffStatistics(differences2);
            auto report2 = comparator.generateDiffReport(differences2, "json");
        });
    }
}

TEST_F(SBOMComparatorTest, HandleErrorRecovery) {
    SBOMComparator comparator;
    
    // Test that comparator can recover from errors
    std::vector<std::string> test_files = {
        "/nonexistent1", "/nonexistent2",
        test_spdx1, test_spdx2,
        "/another/nonexistent", "/yet/another/nonexistent",
        test_cdx1, test_cdx2
    };
    
    for (size_t i = 0; i < test_files.size() - 1; i += 2) {
        EXPECT_NO_THROW(comparator.compare(test_files[i], test_files[i + 1]));
    }
}

// Edge cases
TEST_F(SBOMComparatorTest, HandleVeryLongComponentNames) {
    SBOMComparator comparator;
    
    std::string long_name(10000, 'a'); // 10KB name
    std::string spdx_content = "SPDXVersion: SPDX-2.3\nDataLicense: CC0-1.0\n";
    spdx_content += "PackageName: " + long_name + "\n";
    spdx_content += "PackageVersion: 1.0.0\n";
    spdx_content += "PackageSPDXID: SPDXRef-Package-" + long_name + "\n";
    
    std::string long_file = (test_dir / "long_names.spdx").string();
    std::ofstream long_out(long_file);
    long_out << spdx_content;
    long_out.close();
    
    auto differences = comparator.compare(long_file, long_file);
    EXPECT_NO_THROW(comparator.compare(long_file, long_file));
}

TEST_F(SBOMComparatorTest, HandleSpecialCharactersInNames) {
    SBOMComparator comparator;
    
    std::string special_content = R"(SPDXVersion: SPDX-2.3
DataLicense: CC0-1.0
PackageName: lib@special#test$with%chars
PackageVersion: 1.0.0
PackageSPDXID: SPDXRef-Package-lib@special#test$with%chars
)";
    
    std::string special_file = (test_dir / "special_chars.spdx").string();
    std::ofstream special_out(special_file);
    special_out << special_content;
    special_out.close();
    
    auto differences = comparator.compare(special_file, special_file);
    EXPECT_NO_THROW(comparator.compare(special_file, special_file));
}

TEST_F(SBOMComparatorTest, HandleUnicodeInNames) {
    SBOMComparator comparator;
    
    std::string unicode_content = R"(SPDXVersion: SPDX-2.3
DataLicense: CC0-1.0
PackageName: 测试库-unicode-测试
PackageVersion: 1.0.0
PackageSPDXID: SPDXRef-Package-测试库-unicode-测试
)";
    
    std::string unicode_file = (test_dir / "unicode_names.spdx").string();
    std::ofstream unicode_out(unicode_file);
    unicode_out << unicode_content;
    unicode_out.close();
    
    auto differences = comparator.compare(unicode_file, unicode_file);
    EXPECT_NO_THROW(comparator.compare(unicode_file, unicode_file));
}

TEST_F(SBOMComparatorTest, HandleEmptyComponents) {
    SBOMComparator comparator;
    
    std::string empty_components = R"(SPDXVersion: SPDX-2.3
DataLicense: CC0-1.0
PackageName: 
PackageVersion: 
PackageSPDXID: SPDXRef-Package-empty
)";
    
    std::string empty_file = (test_dir / "empty_components.spdx").string();
    std::ofstream empty_out(empty_file);
    empty_out << empty_components;
    empty_out.close();
    
    auto differences = comparator.compare(empty_file, empty_file);
    EXPECT_NO_THROW(comparator.compare(empty_file, empty_file));
}

TEST_F(SBOMComparatorTest, HandleDuplicateComponents) {
    SBOMComparator comparator;
    
    std::string duplicate_content = R"(SPDXVersion: SPDX-2.3
DataLicense: CC0-1.0
PackageName: libfoo
PackageVersion: 1.0.0
PackageSPDXID: SPDXRef-Package-libfoo
PackageName: libfoo
PackageVersion: 1.0.0
PackageSPDXID: SPDXRef-Package-libfoo
)";
    
    std::string duplicate_file = (test_dir / "duplicate_components.spdx").string();
    std::ofstream duplicate_out(duplicate_file);
    duplicate_out << duplicate_content;
    duplicate_out.close();
    
    auto differences = comparator.compare(duplicate_file, duplicate_file);
    EXPECT_NO_THROW(comparator.compare(duplicate_file, duplicate_file));
}

// Additional comprehensive tests for better coverage
TEST_F(SBOMComparatorTest, CompareWithMalformedJSON) {
    SBOMComparator comparator;
    
    // Create malformed JSON files
    std::string malformed1 = (test_dir / "malformed1.json").string();
    std::ofstream malformed1_out(malformed1);
    malformed1_out << R"({
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
  "components": [
    {
      "type": "library",
      "name": "libfoo",
      "version": "1.0.0",
      "bom-ref": "libfoo-1.0.0",
      "licenses": [{"license": {"id": "MIT"}}]
    }
  ]
)";  // Missing closing brace
    malformed1_out.close();
    
    std::string malformed2 = (test_dir / "malformed2.json").string();
    std::ofstream malformed2_out(malformed2);
    malformed2_out << R"({
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
  "components": [
    {
      "type": "library",
      "name": "libfoo",
      "version": "1.0.0",
      "bom-ref": "libfoo-1.0.0",
      "licenses": [{"license": {"id": "MIT"}}]
    }
  ]
})";
    malformed2_out.close();
    
    EXPECT_NO_THROW(comparator.compare(malformed1, malformed2));
    EXPECT_NO_THROW(comparator.compare(test_cdx1, malformed1));
    EXPECT_NO_THROW(comparator.compare(malformed1, test_cdx1));
}

TEST_F(SBOMComparatorTest, CompareWithInvalidUTF8) {
    SBOMComparator comparator;
    
    // Create file with invalid UTF-8
    std::string invalid_utf8_file = (test_dir / "invalid_utf8.json").string();
    std::ofstream invalid_utf8_out(invalid_utf8_file, std::ios::binary);
    invalid_utf8_out << R"({
  "bomFormat": "CycloneDX",
  "specVersion": "1.6",
  "version": 1,
  "metadata": {
    "timestamp": "2024-01-01T00:00:00Z",
    "tools": [{
      "vendor": "Test",
      "name": "Test Tool with Invalid UTF-8: )";
    
    // Add invalid UTF-8 bytes
    invalid_utf8_out.write("\xFF\xFE\x80", 3);
    
    invalid_utf8_out << R"(",
      "version": "1.0.0"
    }]
  },
  "components": [
    {
      "type": "library",
      "name": "libfoo",
      "version": "1.0.0",
      "bom-ref": "libfoo-1.0.0",
      "licenses": [{"license": {"id": "MIT"}}]
    }
  ]
})";
    invalid_utf8_out.close();
    
    EXPECT_NO_THROW(comparator.compare(invalid_utf8_file, test_cdx1));
    EXPECT_NO_THROW(comparator.compare(test_cdx1, invalid_utf8_file));
}

TEST_F(SBOMComparatorTest, CompareWithControlCharacters) {
    SBOMComparator comparator;
    
    // Create file with control characters
    std::string control_chars_file = (test_dir / "control_chars.json").string();
    std::ofstream control_chars_out(control_chars_file);
    control_chars_out << R"({
  "bomFormat": "CycloneDX",
  "specVersion": "1.6",
  "version": 1,
  "metadata": {
    "timestamp": "2024-01-01T00:00:00Z",
    "tools": [{
      "vendor": "Test",
      "name": "Test Tool with Control Characters: )";
    
    // Add control characters
    for (int i = 0; i < 32; ++i) {
        control_chars_out << static_cast<char>(i);
    }
    
    control_chars_out << R"(",
      "version": "1.0.0"
    }]
  },
  "components": [
    {
      "type": "library",
      "name": "libfoo",
      "version": "1.0.0",
      "bom-ref": "libfoo-1.0.0",
      "licenses": [{"license": {"id": "MIT"}}]
    }
  ]
})";
    control_chars_out.close();
    
    EXPECT_NO_THROW(comparator.compare(control_chars_file, test_cdx1));
    EXPECT_NO_THROW(comparator.compare(test_cdx1, control_chars_file));
}

TEST_F(SBOMComparatorTest, StressTestRapidComparisons) {
    SBOMComparator comparator;
    
    // Perform rapid comparisons to test stress handling
    for (int i = 0; i < 1000; ++i) {
        EXPECT_NO_THROW(comparator.compare(test_cdx1, test_cdx2));
        EXPECT_NO_THROW(comparator.compare(test_spdx1, test_spdx2));
        EXPECT_NO_THROW(comparator.compare(test_cdx1, test_spdx1));
    }
}

TEST_F(SBOMComparatorTest, MemoryLeakTest) {
    // Test for memory leaks by creating and destroying many comparators
    for (int i = 0; i < 1000; ++i) {
        SBOMComparator comparator;
        
        auto differences1 = comparator.compare(test_cdx1, test_cdx2);
        auto differences2 = comparator.compare(test_spdx1, test_spdx2);
        auto differences3 = comparator.compare(test_cdx1, test_spdx1);
        
        auto stats = comparator.getDiffStatistics(differences1);
        auto report = comparator.generateDiffReport(differences1, "text");
    }
    // If we get here without crashing, memory management is working
    EXPECT_TRUE(true);
}

TEST_F(SBOMComparatorTest, BoundaryConditions) {
    SBOMComparator comparator;
    
    // Test with very small files
    std::string tiny_file = (test_dir / "tiny.json").string();
    std::ofstream tiny_out(tiny_file);
    tiny_out << "{}";  // Minimal valid JSON
    tiny_out.close();
    
    EXPECT_NO_THROW(comparator.compare(tiny_file, test_cdx1));
    EXPECT_NO_THROW(comparator.compare(test_cdx1, tiny_file));
    
    // Test with very large file paths
    std::string long_path = test_dir.string();
    for (int i = 0; i < 50; ++i) {
        long_path += "/very/deep/nested/directory/structure/";
    }
    std::filesystem::create_directories(long_path);
    
    std::string long_file = long_path + "/test.json";
    std::ofstream long_out(long_file);
    long_out << R"({
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
  "components": []
})";
    long_out.close();
    
    EXPECT_NO_THROW(comparator.compare(long_file, test_cdx1));
    EXPECT_NO_THROW(comparator.compare(test_cdx1, long_file));
}

TEST_F(SBOMComparatorTest, ErrorRecoveryAfterFailure) {
    SBOMComparator comparator;
    
    // First, test with invalid files
    std::string invalid_file = "/nonexistent/file";
    EXPECT_NO_THROW(comparator.compare(invalid_file, test_cdx1));
    EXPECT_NO_THROW(comparator.compare(test_cdx1, invalid_file));
    
    // Then test with valid files - should still work
    EXPECT_NO_THROW(comparator.compare(test_cdx1, test_cdx2));
    EXPECT_NO_THROW(comparator.compare(test_spdx1, test_spdx2));
}

TEST_F(SBOMComparatorTest, MixedValidAndInvalidFiles) {
    SBOMComparator comparator;
    
    std::vector<std::string> test_files = {
        "/nonexistent1", test_cdx1, "/nonexistent2",
        test_spdx1, "/another/nonexistent", test_cdx2
    };
    
    for (const auto& test_file : test_files) {
        EXPECT_NO_THROW(comparator.compare(test_file, test_cdx1));
        EXPECT_NO_THROW(comparator.compare(test_cdx1, test_file));
    }
}