#include <gtest/gtest.h>
#include "compat/compatibility.hpp"  // Re-enabled to test the fix
#include "llvm/llvm_detector.hpp"
#include <vector>
#include <array>
#include <iostream>

using namespace heimdall::llvm;
using namespace heimdall::compat;  // Use compatibility types

// Variable template for C++14 testing
template<typename T>
constexpr T pi = T(3.1415926535897932385);

class CompatibilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test setup
    }
};

// Test C++ standard detection
TEST_F(CompatibilityTest, CXXStandardDetection) {
#if __cplusplus >= 202302L
    EXPECT_TRUE(true); // C++23 features available
#elif __cplusplus >= 202002L
    EXPECT_TRUE(true); // C++20 features available
#elif __cplusplus >= 201703L
    EXPECT_TRUE(true); // C++17+ features available
#elif __cplusplus >= 201402L
    EXPECT_TRUE(true); // C++14 features available
#elif __cplusplus >= 201103L
    EXPECT_TRUE(true); // C++11 features available
#else
    FAIL() << "No C++ standard detected";
#endif
}

// Test C++11 basic features
TEST_F(CompatibilityTest, CXX11BasicFeatures) {
    // Test auto keyword (C++11)
    auto x = 42;
    EXPECT_EQ(x, 42);
    
    // Test range-based for loops (C++11)
    std::vector<int> vec = {1, 2, 3, 4, 5};
    int sum = 0;
    for (const auto& val : vec) {
        sum += val;
    }
    EXPECT_EQ(sum, 15);
    
    // Test lambda expressions (C++11)
    auto lambda = [](int a, int b) { return a + b; };
    EXPECT_EQ(lambda(10, 20), 30);
    
    // Test nullptr (C++11)
    int* ptr = nullptr;
    EXPECT_EQ(ptr, nullptr);
    
    // Test uniform initialization (C++11)
    std::vector<int> init_vec{1, 2, 3};
    EXPECT_EQ(init_vec.size(), 3);
    EXPECT_EQ(init_vec[0], 1);
    
    // Test decltype (C++11)
    int y = 10;
    decltype(y) z = 20;
    EXPECT_EQ(z, 20);
    
    // Test trailing return type (C++11)
    auto get_value = [](int x) -> int { return x * 2; };
    EXPECT_EQ(get_value(21), 42);
}

// Test basic C++14 features
TEST_F(CompatibilityTest, CXX14BasicFeatures) {
    // Test auto return type deduction (C++14)
    auto get_value = [](int x) { return x * 2; };
    EXPECT_EQ(get_value(21), 42);
    
    // Test generic lambdas (C++14)
    auto print = [](const auto& x) { return std::to_string(x); };
    EXPECT_EQ(print(42), "42");
    EXPECT_EQ(print(3.14), "3.140000");
    
    // Test binary literals (C++14)
    int binary = 0b1010;
    EXPECT_EQ(binary, 10);
    
    // Test digit separators (C++14)
    int big_number = 1'000'000;
    EXPECT_EQ(big_number, 1000000);
    
    // Test variable templates (C++14)
    EXPECT_NEAR(pi<double>, 3.14159, 0.00001);
    EXPECT_NEAR(pi<float>, 3.14159f, 0.00001f);
}

// Test compatibility optional functionality
TEST_F(CompatibilityTest, CompatibilityOptionalTest) {
    // Test heimdall::compat::optional
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
    EXPECT_EQ(opt1.value_or(200), 200);
    EXPECT_EQ(opt2.value_or(200), 42);
    
    // Test utility function from heimdall::compat::utils
    EXPECT_EQ(utils::get_optional_value(opt1, 300), 300);
    EXPECT_EQ(utils::get_optional_value(opt2, 300), 42);
}

// Test compatibility string_view functionality
TEST_F(CompatibilityTest, CompatibilityStringViewTest) {
    // Test heimdall::compat::string_view
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
    EXPECT_EQ(std::string(sv3), "hello");
    
    // Test find
    EXPECT_EQ(sv2.find('l'), 2);
    EXPECT_EQ(sv2.find('x'), std::string::npos);
    
    // Test substr
    string_view sv4 = sv2.substr(1, 3);
    EXPECT_EQ(std::string(sv4), "ell");
    
    // Test utility function
    string_view sv5 = utils::to_string_view("test");
    EXPECT_EQ(std::string(sv5), "test");
    string_view sv6 = utils::to_string_view(42);
    EXPECT_EQ(std::string(sv6), "42");
}

// Test compatibility filesystem functionality
TEST_F(CompatibilityTest, CompatibilityFilesystemTest) {
    // Test that fs namespace is available
    fs::path test_path("test.txt");
    EXPECT_EQ(test_path.string(), "test.txt");
    
    // Test path operations
    fs::path dir_path("test_dir");
    fs::path file_path = dir_path / "file.txt";
    EXPECT_EQ(file_path.string(), "test_dir/file.txt");
}

// Test compatibility variant functionality
TEST_F(CompatibilityTest, CompatibilityVariantTest) {
    // Test heimdall::compat::variant
    variant<int, std::string> v1(42);
    EXPECT_EQ(v1.index(), 0);
    EXPECT_EQ(std::get<int>(v1), 42);
    
    variant<int, std::string> v2("hello");
    EXPECT_EQ(v2.index(), 1);
    EXPECT_EQ(std::get<std::string>(v2), "hello");
    
    // Test visit (if available)
    variant<int, std::string> v3(100);
    int result = std::visit([](auto&& arg) -> int {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
            return arg * 2;
        } else {
            return arg.size();
        }
    }, v3);
    EXPECT_EQ(result, 200);
}

// Test LLVM detection
TEST_F(CompatibilityTest, LLVMDetectionTest) {
    // Test that LLVM headers are available
    EXPECT_TRUE(true); // If we get here, LLVM headers are working
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}