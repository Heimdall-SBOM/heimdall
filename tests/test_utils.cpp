#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "Utils.hpp"

using namespace heimdall;

class UtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = std::filesystem::temp_directory_path() / "heimdall_utils_test";
        std::filesystem::create_directories(test_dir);
        test_file = test_dir / "file.txt";
        std::ofstream(test_file) << "test content";
    }
    void TearDown() override {
        std::filesystem::remove_all(test_dir);
    }
    std::filesystem::path test_dir;
    std::filesystem::path test_file;
};

TEST_F(UtilsTest, GetDirectory) {
    std::string dir = Utils::getDirectory(test_file.string());
    EXPECT_EQ(dir, test_dir.string());
}

TEST_F(UtilsTest, GetFileName) {
    std::string name = Utils::getFileName(test_file.string());
    EXPECT_EQ(name, "file.txt");
}

TEST_F(UtilsTest, GetFileExtension) {
    std::string ext = Utils::getFileExtension(test_file.string());
    EXPECT_EQ(ext, ".txt");
}

TEST_F(UtilsTest, FileExists) {
    EXPECT_TRUE(Utils::fileExists(test_file.string()));
    EXPECT_FALSE(Utils::fileExists((test_dir / "nonexistent.txt").string()));
}

TEST_F(UtilsTest, IsSystemLibrary) {
    // This is a heuristic; just check it doesn't crash
    EXPECT_FALSE(Utils::isSystemLibrary(test_file.string()));
}

TEST(UtilsJson, EscapeJsonString) {
    std::string raw = "Heimdall\n\"SBOM\"";
    std::string escaped = Utils::escapeJsonString(raw);
    EXPECT_NE(escaped.find("\\n"), std::string::npos);
    EXPECT_NE(escaped.find("\\\""), std::string::npos);
}

TEST(UtilsJson, FormatJsonValue) {
    std::string value = "test\nvalue";
    std::string formatted = Utils::formatJsonValue(value);
    EXPECT_TRUE(formatted.front() == '"' && formatted.back() == '"');
} 
 