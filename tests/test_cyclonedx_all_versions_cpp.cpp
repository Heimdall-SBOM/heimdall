#include <gtest/gtest.h>
#include "compat/compatibility.hpp"
#include "llvm/llvm_detector.hpp"

using namespace heimdall::compat;
using namespace heimdall::llvm;

class CompatibilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test setup
    }
};

// Test C++ standard detection
TEST_F(CompatibilityTest, CXXStandardDetection) {
#if HEIMDALL_CPP17_AVAILABLE
    EXPECT_TRUE(true); // C++17+ features available
#elif HEIMDALL_CPP14_AVAILABLE
    EXPECT_TRUE(true); // C++14 features available
#elif HEIMDALL_CPP11_AVAILABLE
    EXPECT_TRUE(true); // C++11 features available
#else
    FAIL() << "No C++ standard detected";
#endif
}

// Test optional functionality
TEST_F(CompatibilityTest, OptionalTest) {
    optional<int> opt1;
    EXPECT_FALSE(opt1.has_value());
    
    optional<int> opt2(42);
    EXPECT_TRUE(opt2.has_value());
    EXPECT_EQ(opt2.value(), 42);
    EXPECT_EQ(*opt2, 42);
    
    optional<int> opt3 = opt2;
    EXPECT_TRUE(opt3.has_value());
    EXPECT_EQ(opt3.value(), 42);
    
    // Test value_or
    EXPECT_EQ(opt1.value_or(100), 100);
    EXPECT_EQ(opt2.value_or(100), 42);
}

// Test string_view functionality
TEST_F(CompatibilityTest, StringViewTest) {
    string_view sv1;
    EXPECT_TRUE(sv1.empty());
    EXPECT_EQ(sv1.size(), 0);
    
    string_view sv2("hello");
    EXPECT_FALSE(sv2.empty());
    EXPECT_EQ(sv2.size(), 5);
    EXPECT_EQ(sv2[0], 'h');
    EXPECT_EQ(sv2[4], 'o');
    
    string_view sv3("hello world", 5);
    EXPECT_EQ(sv3.size(), 5);
    EXPECT_EQ(sv3.to_string(), "hello");
    
    // Test find
    EXPECT_EQ(sv2.find('l'), 2);
    EXPECT_EQ(sv2.find('x'), std::string::npos);
    
    // Test substr
    string_view sv4 = sv2.substr(1, 3);
    EXPECT_EQ(sv4.to_string(), "ell");
}

// Test filesystem compatibility
TEST_F(CompatibilityTest, FilesystemTest) {
    // Test that fs namespace is available
    fs::path test_path("test.txt");
    EXPECT_EQ(test_path.string(), "test.txt");
    
    // Test path operations
    fs::path dir_path("test_dir");
    fs::path file_path = dir_path / "file.txt";
    EXPECT_EQ(file_path.string(), "test_dir/file.txt");
}

// Test LLVM version detection
TEST_F(CompatibilityTest, LLVMDetectionTest) {
    LLVMVersion version = LLVMDetector::detectVersion();
    
    // Version should be one of the known versions or UNKNOWN
    EXPECT_TRUE(version == LLVMVersion::UNKNOWN ||
                version == LLVMVersion::LLVM_7_10 ||
                version == LLVMVersion::LLVM_11_18 ||
                version == LLVMVersion::LLVM_19_PLUS);
    
    // Test version string
    std::string version_str = LLVMDetector::getVersionString(version);
    EXPECT_FALSE(version_str.empty());
    
    // Test supported standards
    std::vector<int> standards = LLVMDetector::getSupportedCXXStandards(version);
    if (version != LLVMVersion::UNKNOWN) {
        EXPECT_FALSE(standards.empty());
    }
}

// Test C++ standard compatibility
TEST_F(CompatibilityTest, CXXStandardCompatibilityTest) {
    LLVMVersion version = LLVMDetector::detectVersion();
    
    // Test C++11 support
    bool supports_cpp11 = LLVMDetector::supportsCXXStandard(version, 11);
    if (version != LLVMVersion::UNKNOWN) {
        EXPECT_TRUE(supports_cpp11);
    }
    
    // Test C++14 support
    bool supports_cpp14 = LLVMDetector::supportsCXXStandard(version, 14);
    if (version != LLVMVersion::UNKNOWN) {
        EXPECT_TRUE(supports_cpp14);
    }
    
    // Test C++17 support
    bool supports_cpp17 = LLVMDetector::supportsCXXStandard(version, 17);
    if (version == LLVMVersion::LLVM_11_18 || version == LLVMVersion::LLVM_19_PLUS) {
        EXPECT_TRUE(supports_cpp17);
    }
    
    // Test C++20 support
    bool supports_cpp20 = LLVMDetector::supportsCXXStandard(version, 20);
    if (version == LLVMVersion::LLVM_19_PLUS) {
        EXPECT_TRUE(supports_cpp20);
    }
}

// Test DWARF support
TEST_F(CompatibilityTest, DWARFSupportTest) {
    LLVMVersion version = LLVMDetector::detectVersion();
    bool supports_dwarf = LLVMDetector::supportsDWARF(version);
    
    if (version != LLVMVersion::UNKNOWN) {
        EXPECT_TRUE(supports_dwarf);
    }
}

// Test minimum LLVM version requirements
TEST_F(CompatibilityTest, MinimumLLVMVersionTest) {
    LLVMVersion min_cpp11 = LLVMDetector::getMinimumLLVMVersion(11);
    EXPECT_EQ(min_cpp11, LLVMVersion::LLVM_7_10);
    
    LLVMVersion min_cpp14 = LLVMDetector::getMinimumLLVMVersion(14);
    EXPECT_EQ(min_cpp14, LLVMVersion::LLVM_7_10);
    
    LLVMVersion min_cpp17 = LLVMDetector::getMinimumLLVMVersion(17);
    EXPECT_EQ(min_cpp17, LLVMVersion::LLVM_11_18);
    
    LLVMVersion min_cpp20 = LLVMDetector::getMinimumLLVMVersion(20);
    EXPECT_EQ(min_cpp20, LLVMVersion::LLVM_19_PLUS);
    
    LLVMVersion min_cpp23 = LLVMDetector::getMinimumLLVMVersion(23);
    EXPECT_EQ(min_cpp23, LLVMVersion::LLVM_19_PLUS);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}