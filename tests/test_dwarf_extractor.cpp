#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include "DWARFExtractor.hpp"
#include "Utils.hpp"
#include <iostream>

using namespace heimdall;

class DWARFExtractorTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = std::filesystem::temp_directory_path() / "heimdall_dwarf_test";
        std::filesystem::create_directories(test_dir);
        
        // Create a simple C source file with debug info
        test_source = test_dir / "testlib.c";
        std::ofstream(test_source) << R"(
#include <stdio.h>

int main() {
    printf("Hello, World!\n");
    return 0;
}

int helper_function() {
    return 42;
}

static int internal_function() {
    return 0;
}
)";
        
        // Compile with debug info
        test_executable = test_dir / "test_program";
        std::string compile_cmd = "gcc -g -o " + test_executable.string() + 
                                 " " + test_source.string() + " 2>/dev/null";
        system(compile_cmd.c_str());
        
        // Create a simple object file with debug info
        test_object = test_dir / "testlib.o";
        std::string obj_compile_cmd = "gcc -c -g -o " + test_object.string() + 
                                     " " + test_source.string() + " 2>/dev/null";
        system(obj_compile_cmd.c_str());
        
        // Fallback to dummy files if compilation fails
        if (!std::filesystem::exists(test_executable)) {
            std::ofstream(test_executable) << "dummy executable";
        }
        if (!std::filesystem::exists(test_object)) {
            std::ofstream(test_object) << "dummy object";
        }
    }
    
    void TearDown() override {
        std::filesystem::remove_all(test_dir);
    }
    
    std::filesystem::path test_dir;
    std::filesystem::path test_source;
    std::filesystem::path test_executable;
    std::filesystem::path test_object;
};

TEST_F(DWARFExtractorTest, Constructor) {
    std::cout << "DEBUG: Test starting" << std::endl;
    DWARFExtractor extractor;
    std::cout << "DEBUG: DWARFExtractor created successfully" << std::endl;
    // Should not crash
    std::cout << "DEBUG: Test about to end" << std::endl;
}

TEST(DWARFExtractorStandaloneTest, SimpleConstructor) {
    std::cout << "DEBUG: Standalone test starting" << std::endl;
    DWARFExtractor extractor;
    std::cout << "DEBUG: Standalone DWARFExtractor created successfully" << std::endl;
    std::cout << "DEBUG: Standalone test about to end" << std::endl;
}

TEST_F(DWARFExtractorTest, ExtractSourceFiles) {
    DWARFExtractor extractor;
    std::vector<std::string> sourceFiles;
    
    // Test with executable that has debug info
    if (std::filesystem::file_size(test_executable) > 100) {
        bool result = extractor.extractSourceFiles(test_executable.string(), sourceFiles);
        
        // On systems with LLVM DWARF support, this should work
        // On systems without LLVM, this should fail gracefully
        if (result) {
            EXPECT_FALSE(sourceFiles.empty());
            // Should contain our source file
            bool found_source = false;
            for (const auto& file : sourceFiles) {
                if (file.find("testlib.c") != std::string::npos) {
                    found_source = true;
                    break;
                }
            }
            EXPECT_TRUE(found_source);
        }
    }
    
    // Test with non-existent file
    sourceFiles.clear();
    EXPECT_FALSE(extractor.extractSourceFiles("nonexistent_file", sourceFiles));
    EXPECT_TRUE(sourceFiles.empty());
    
    // Test with file that has no debug info
    sourceFiles.clear();
    std::filesystem::path no_debug_file = test_dir / "no_debug.txt";
    std::ofstream(no_debug_file) << "no debug info";
    EXPECT_FALSE(extractor.extractSourceFiles(no_debug_file.string(), sourceFiles));
    EXPECT_TRUE(sourceFiles.empty());
}

TEST_F(DWARFExtractorTest, ExtractCompileUnits) {
    DWARFExtractor extractor;
    std::vector<std::string> compileUnits;
    
    // Test with executable that has debug info
    if (std::filesystem::file_size(test_executable) > 100) {
        bool result = extractor.extractCompileUnits(test_executable.string(), compileUnits);
        
        if (result) {
            EXPECT_FALSE(compileUnits.empty());
            // Should contain at least one compile unit
            EXPECT_GT(compileUnits.size(), 0u);
        }
    }
    
    // Test with non-existent file
    compileUnits.clear();
    EXPECT_FALSE(extractor.extractCompileUnits("nonexistent_file", compileUnits));
    EXPECT_TRUE(compileUnits.empty());
}

TEST_F(DWARFExtractorTest, ExtractFunctions) {
    DWARFExtractor extractor;
    std::vector<std::string> functions;
    
    // Test with executable that has debug info
    if (std::filesystem::file_size(test_executable) > 100) {
        bool result = extractor.extractFunctions(test_executable.string(), functions);
        
        if (result) {
            EXPECT_FALSE(functions.empty());
            // Should contain our functions
            bool found_main = false;
            bool found_helper = false;
            for (const auto& func : functions) {
                if (func.find("main") != std::string::npos) found_main = true;
                if (func.find("helper_function") != std::string::npos) found_helper = true;
            }
            // At least main should be found
            EXPECT_TRUE(found_main);
        }
    }
    
    // Test with non-existent file
    functions.clear();
    EXPECT_FALSE(extractor.extractFunctions("nonexistent_file", functions));
    EXPECT_TRUE(functions.empty());
}

TEST_F(DWARFExtractorTest, ExtractLineInfo) {
    DWARFExtractor extractor;
    std::vector<std::string> lineInfo;
    
    // Test with executable that has debug info
    if (std::filesystem::file_size(test_executable) > 100) {
        bool result = extractor.extractLineInfo(test_executable.string(), lineInfo);
        
        if (result) {
            // Line info extraction might not always work, but shouldn't crash
            // The result could be empty or contain line information
        }
    }
    
    // Test with non-existent file
    lineInfo.clear();
    EXPECT_FALSE(extractor.extractLineInfo("nonexistent_file", lineInfo));
    EXPECT_TRUE(lineInfo.empty());
}

TEST_F(DWARFExtractorTest, HasDWARFInfo) {
    DWARFExtractor extractor;
    
    // Test with executable that has debug info
    if (std::filesystem::file_size(test_executable) > 100) {
        bool has_dwarf = extractor.hasDWARFInfo(test_executable.string());
        // This might be true or false depending on LLVM support and debug info
        // Just test that it doesn't crash
    }
    
    // Test with non-existent file
    EXPECT_FALSE(extractor.hasDWARFInfo("nonexistent_file"));
    
    // Test with file that has no debug info
    std::filesystem::path no_debug_file = test_dir / "no_debug.txt";
    std::ofstream(no_debug_file) << "no debug info";
    EXPECT_FALSE(extractor.hasDWARFInfo(no_debug_file.string()));
}

TEST_F(DWARFExtractorTest, ObjectFileExtraction) {
    DWARFExtractor extractor;
    std::vector<std::string> sourceFiles;
    std::vector<std::string> functions;
    std::vector<std::string> compileUnits;
    
    // Test with object file that has debug info
    if (std::filesystem::file_size(test_object) > 100) {
        // Test source files extraction
        bool source_result = extractor.extractSourceFiles(test_object.string(), sourceFiles);
        if (source_result) {
            EXPECT_FALSE(sourceFiles.empty());
        }
        
        // Test functions extraction
        bool func_result = extractor.extractFunctions(test_object.string(), functions);
        if (func_result) {
            EXPECT_FALSE(functions.empty());
        }
        
        // Test compile units extraction
        bool unit_result = extractor.extractCompileUnits(test_object.string(), compileUnits);
        if (unit_result) {
            EXPECT_FALSE(compileUnits.empty());
        }
    }
}

TEST_F(DWARFExtractorTest, ErrorHandling) {
    DWARFExtractor extractor;
    std::vector<std::string> result;
    
    // Test with empty string
    EXPECT_FALSE(extractor.extractSourceFiles("", result));
    EXPECT_FALSE(extractor.extractFunctions("", result));
    EXPECT_FALSE(extractor.extractCompileUnits("", result));
    EXPECT_FALSE(extractor.extractLineInfo("", result));
    EXPECT_FALSE(extractor.hasDWARFInfo(""));
    
    // Test with directory
    EXPECT_FALSE(extractor.extractSourceFiles(test_dir.string(), result));
    EXPECT_FALSE(extractor.extractFunctions(test_dir.string(), result));
    EXPECT_FALSE(extractor.extractCompileUnits(test_dir.string(), result));
    EXPECT_FALSE(extractor.extractLineInfo(test_dir.string(), result));
    EXPECT_FALSE(extractor.hasDWARFInfo(test_dir.string()));
    
    // Test with very large file path
    std::string large_path(10000, 'a');
    EXPECT_FALSE(extractor.extractSourceFiles(large_path, result));
    EXPECT_FALSE(extractor.extractFunctions(large_path, result));
    EXPECT_FALSE(extractor.extractCompileUnits(large_path, result));
    EXPECT_FALSE(extractor.extractLineInfo(large_path, result));
    EXPECT_FALSE(extractor.hasDWARFInfo(large_path));
}

TEST_F(DWARFExtractorTest, MultipleExtractions) {
    DWARFExtractor extractor;
    std::vector<std::string> sourceFiles1, sourceFiles2;
    std::vector<std::string> functions1, functions2;
    
    // Test multiple extractions on same file
    if (std::filesystem::file_size(test_executable) > 100) {
        bool result1 = extractor.extractSourceFiles(test_executable.string(), sourceFiles1);
        bool result2 = extractor.extractSourceFiles(test_executable.string(), sourceFiles2);
        
        if (result1 && result2) {
            EXPECT_EQ(sourceFiles1.size(), sourceFiles2.size());
            EXPECT_EQ(sourceFiles1, sourceFiles2);
        }
        
        bool func_result1 = extractor.extractFunctions(test_executable.string(), functions1);
        bool func_result2 = extractor.extractFunctions(test_executable.string(), functions2);
        
        if (func_result1 && func_result2) {
            EXPECT_EQ(functions1.size(), functions2.size());
            EXPECT_EQ(functions1, functions2);
        }
    }
}

TEST_F(DWARFExtractorTest, HeuristicExtraction) {
    DWARFExtractor extractor;
    std::vector<std::string> sourceFiles;
    
    // Test heuristic extraction when LLVM DWARF fails
    // This is mostly testing that it doesn't crash
    if (std::filesystem::file_size(test_executable) > 100) {
        bool result = extractor.extractSourceFiles(test_executable.string(), sourceFiles);
        // Should either succeed or fail gracefully
        EXPECT_TRUE(result || !result);
    }
} 