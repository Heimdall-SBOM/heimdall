#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "SBOMGenerator.hpp"
#include "ComponentInfo.hpp"

using namespace heimdall;

class SBOMGeneratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = std::filesystem::temp_directory_path() / "heimdall_sbom_test";
        std::filesystem::create_directories(test_dir);
        test_file = test_dir / "libfoo.so";
        std::ofstream(test_file) << "dummy content";
    }
    void TearDown() override {
        std::filesystem::remove_all(test_dir);
    }
    std::filesystem::path test_dir;
    std::filesystem::path test_file;
};

TEST_F(SBOMGeneratorTest, ProcessComponentAndCount) {
    SBOMGenerator generator;
    ComponentInfo component("foo", test_file.string());
    generator.processComponent(component);
    EXPECT_EQ(generator.getComponentCount(), 1u);
    EXPECT_TRUE(generator.hasComponent("foo"));
}

TEST_F(SBOMGeneratorTest, SetOutputPathAndFormat) {
    SBOMGenerator generator;
    generator.setOutputPath((test_dir / "sbom.spdx").string());
    generator.setFormat("spdx");
    // No direct way to check, but should not crash
    generator.printStatistics();
}

TEST_F(SBOMGeneratorTest, GenerateSBOMSPDX) {
    SBOMGenerator generator;
    generator.setOutputPath((test_dir / "sbom.spdx").string());
    generator.setFormat("spdx");
    ComponentInfo component("foo", test_file.string());
    generator.processComponent(component);
    generator.generateSBOM();
    std::ifstream f(test_dir / "sbom.spdx");
    std::stringstream buffer;
    buffer << f.rdbuf();
    std::string content = buffer.str();
    EXPECT_NE(content.find("SPDXVersion"), std::string::npos);
    EXPECT_NE(content.find("foo"), std::string::npos);
}

TEST_F(SBOMGeneratorTest, GenerateSBOMCycloneDX) {
    SBOMGenerator generator;
    generator.setOutputPath((test_dir / "sbom.cdx.json").string());
    generator.setFormat("cyclonedx");
    ComponentInfo component("foo", test_file.string());
    generator.processComponent(component);
    generator.generateSBOM();
    std::ifstream f(test_dir / "sbom.cdx.json");
    std::stringstream buffer;
    buffer << f.rdbuf();
    std::string content = buffer.str();
    EXPECT_NE(content.find("CycloneDX"), std::string::npos);
    EXPECT_NE(content.find("foo"), std::string::npos);
} 