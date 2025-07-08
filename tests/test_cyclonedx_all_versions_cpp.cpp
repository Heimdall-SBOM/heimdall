#include <gtest/gtest.h>
#include "compat/compatibility.hpp"
#include "llvm/llvm_detector.hpp"
#include <vector>
#include <array>

using namespace heimdall::compat;
using namespace heimdall::llvm;
using namespace heimdall::compat::utils;

class CompatibilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test setup
    }
};

// Test C++ standard detection
TEST_F(CompatibilityTest, CXXStandardDetection) {
#if HEIMDALL_CPP23_AVAILABLE
    EXPECT_TRUE(true); // C++23 features available
#elif HEIMDALL_CPP20_AVAILABLE
    EXPECT_TRUE(true); // C++20 features available
#elif HEIMDALL_CPP17_AVAILABLE
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
    
    // Test utility function
    EXPECT_EQ(get_optional_value(opt1, 200), 200);
    EXPECT_EQ(get_optional_value(opt2, 200), 42);
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
    
    // Test utility function
    string_view sv5 = to_string_view("test");
    EXPECT_EQ(sv5.to_string(), "test");
    
    string_view sv6 = to_string_view(42);
    EXPECT_EQ(sv6.to_string(), "42");
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

// Test span functionality
TEST_F(CompatibilityTest, SpanTest) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    span<int> sp(vec);
    
    EXPECT_EQ(sp.size(), 5);
    EXPECT_FALSE(sp.empty());
    EXPECT_EQ(sp[0], 1);
    EXPECT_EQ(sp[4], 5);
    EXPECT_EQ(sp.front(), 1);
    EXPECT_EQ(sp.back(), 5);
    
    // Test iteration
    int sum = 0;
    for (int val : sp) {
        sum += val;
    }
    EXPECT_EQ(sum, 15);
    
    // Test array span
    std::array<int, 3> arr = {10, 20, 30};
    span<int> sp2(arr);
    EXPECT_EQ(sp2.size(), 3);
    EXPECT_EQ(sp2[1], 20);
}

#if HEIMDALL_CPP20_AVAILABLE
// Test C++20 concepts
TEST_F(CompatibilityTest, ConceptsTest) {
    // Test integral concept
    EXPECT_TRUE(integral<int>);
    EXPECT_TRUE(integral<long>);
    EXPECT_FALSE(integral<double>);
    
    // Test floating_point concept
    EXPECT_TRUE(floating_point<double>);
    EXPECT_TRUE(floating_point<float>);
    EXPECT_FALSE(floating_point<int>);
    
    // Test arithmetic concept
    EXPECT_TRUE(arithmetic<int>);
    EXPECT_TRUE(arithmetic<double>);
    EXPECT_FALSE(arithmetic<std::string>);
    
    // Test convertible_to_string concept
    EXPECT_TRUE(convertible_to_string<int>);
    EXPECT_TRUE(convertible_to_string<double>);
    EXPECT_FALSE(convertible_to_string<std::vector<int>>);
}

// Test C++20 ranges
TEST_F(CompatibilityTest, RangesTest) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    // Test ranges::size
    EXPECT_EQ(ranges::size(vec), 5);
    
    // Test ranges::begin and ranges::end
    EXPECT_EQ(*ranges::begin(vec), 1);
    EXPECT_EQ(*(ranges::end(vec) - 1), 5);
    
    // Test ranges::empty
    EXPECT_FALSE(ranges::empty(vec));
    
    std::vector<int> empty_vec;
    EXPECT_TRUE(ranges::empty(empty_vec));
}

// Test C++20 format
TEST_F(CompatibilityTest, FormatTest) {
    std::string result = fmt::format("Hello, {}!", "World");
    EXPECT_EQ(result, "Hello, World!");
    
    result = fmt::format("Number: {}", 42);
    EXPECT_EQ(result, "Number: 42");
    
    result = fmt::format("Float: {:.2f}", 3.14159);
    EXPECT_EQ(result, "Float: 3.14");
}

// Test C++20 expected
TEST_F(CompatibilityTest, ExpectedTest) {
    expected<int, std::string> exp1(42);
    EXPECT_TRUE(exp1.has_value());
    EXPECT_EQ(exp1.value(), 42);
    EXPECT_EQ(*exp1, 42);
    
    expected<int, std::string> exp2(unexpected<std::string>("error"));
    EXPECT_FALSE(exp2.has_value());
    EXPECT_EQ(exp2.error(), "error");
    
    // Test value_or for expected
    EXPECT_EQ(exp1.value_or(100), 42);
    EXPECT_EQ(exp2.value_or(100), 100);
}

// Test C++20 source_location
TEST_F(CompatibilityTest, SourceLocationTest) {
    source_location loc = source_location::current();
    EXPECT_FALSE(loc.file_name().empty());
    EXPECT_GT(loc.line(), 0);
    EXPECT_GT(loc.column(), 0);
}

// Test C++20 three-way comparison
TEST_F(CompatibilityTest, ThreeWayComparisonTest) {
    int a = 5, b = 3, c = 5;
    
    EXPECT_EQ(a <=> b, strong_ordering::greater);
    EXPECT_EQ(b <=> a, strong_ordering::less);
    EXPECT_EQ(a <=> c, strong_ordering::equal);
    
    EXPECT_TRUE(a > b);
    EXPECT_TRUE(b < a);
    EXPECT_TRUE(a == c);
}
#endif // HEIMDALL_CPP20_AVAILABLE

#if HEIMDALL_CPP23_AVAILABLE
// Test C++23 features
TEST_F(CompatibilityTest, CXX23FeaturesTest) {
    // Test to_underlying
    enum class Color { Red = 1, Green = 2, Blue = 3 };
    EXPECT_EQ(to_underlying(Color::Red), 1);
    EXPECT_EQ(to_underlying(Color::Green), 2);
    EXPECT_EQ(to_underlying(Color::Blue), 3);
    
    // Test enum_to_string utility
    std::string color_str = enum_to_string(Color::Red);
    EXPECT_EQ(color_str, "1");
    
    // Test flat_map (if available)
    #if __has_include(<flat_map>)
    flat_map<int, std::string> fm;
    fm.insert({1, "one"});
    fm.insert({2, "two"});
    
    EXPECT_EQ(fm.size(), 2);
    EXPECT_EQ(fm[1], "one");
    EXPECT_EQ(fm[2], "two");
    #endif
    
    // Test flat_set (if available)
    #if __has_include(<flat_set>)
    flat_set<int> fs;
    fs.insert(1);
    fs.insert(2);
    fs.insert(1); // Duplicate
    
    EXPECT_EQ(fs.size(), 2); // Duplicate ignored
    EXPECT_TRUE(fs.contains(1));
    EXPECT_TRUE(fs.contains(2));
    #endif
}

// Test C++23 concepts
TEST_F(CompatibilityTest, CXX23ConceptsTest) {
    std::vector<int> vec = {1, 2, 3};
    std::array<int, 3> arr = {1, 2, 3};
    
    EXPECT_TRUE(sized_range<decltype(vec)>);
    EXPECT_TRUE(sized_range<decltype(arr)>);
    
    EXPECT_TRUE(random_access_range<decltype(vec)>);
    EXPECT_TRUE(random_access_range<decltype(arr)>);
    
    EXPECT_TRUE(contiguous_range<decltype(vec)>);
    EXPECT_TRUE(contiguous_range<decltype(arr)>);
}
#endif // HEIMDALL_CPP23_AVAILABLE

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
    
    // Test C++23 support
    bool supports_cpp23 = LLVMDetector::supportsCXXStandard(version, 23);
    if (version == LLVMVersion::LLVM_19_PLUS) {
        EXPECT_TRUE(supports_cpp23);
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

// Test utility functions
TEST_F(CompatibilityTest, UtilityFunctionsTest) {
    // Test format_string
    std::string result = format_string("Hello, {}!", "World");
    #if HEIMDALL_CPP20_AVAILABLE
    EXPECT_EQ(result, "Hello, World!");
    #else
    EXPECT_FALSE(result.empty());
    #endif
    
    // Test enum_to_string
    enum class TestEnum { Value1 = 10, Value2 = 20 };
    std::string enum_str = enum_to_string(TestEnum::Value1);
    EXPECT_EQ(enum_str, "10");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}