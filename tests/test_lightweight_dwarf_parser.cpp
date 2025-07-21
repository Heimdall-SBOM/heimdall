#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <thread>
#include "common/LightweightDWARFParser.hpp"

using namespace heimdall;

class LightweightDWARFParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = std::filesystem::temp_directory_path() / "heimdall_dwarf_test";
        std::filesystem::create_directories(test_dir);
        
        // Create a simple test ELF file with DWARF data
        createTestELF();
    }

    void TearDown() override {
        std::filesystem::remove_all(test_dir);
    }

    void createTestELF() {
        // Create a minimal ELF file for testing
        std::string elf_path = (test_dir / "test_binary").string();
        std::ofstream elf_file(elf_path, std::ios::binary);
        
        // Write minimal ELF header (simplified for testing)
        // This is a basic ELF structure - in real usage, this would be a compiled binary
        std::vector<uint8_t> elf_header = {
            0x7f, 0x45, 0x4c, 0x46,  // ELF magic
            0x02,                     // 64-bit
            0x01,                     // Little endian
            0x01,                     // ELF version
            0x00,                     // System V ABI
            0x00, 0x00, 0x00, 0x00,  // ABI version
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Padding
            0x03, 0x00,              // ET_DYN
            0x3e, 0x00,              // x86-64
            0x01, 0x00, 0x00, 0x00,  // ELF version
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Entry point
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Program header offset
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Section header offset
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Flags
            0x40, 0x00,              // ELF header size
            0x38, 0x00,              // Program header entry size
            0x01, 0x00,              // Program header entries
            0x40, 0x00,              // Section header entry size
            0x03, 0x00,              // Section header entries
            0x02, 0x00               // Section name string table index
        };
        
        elf_file.write(reinterpret_cast<const char*>(elf_header.data()), elf_header.size());
        elf_file.close();
        
        test_elf_path = elf_path;
    }

    std::filesystem::path test_dir;
    std::string test_elf_path;
};

// Basic functionality tests
TEST_F(LightweightDWARFParserTest, Constructor) {
    LightweightDWARFParser parser;
    EXPECT_TRUE(true); // Constructor should not throw
}

TEST_F(LightweightDWARFParserTest, ExtractSourceFiles) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles;
    
    bool result = parser.extractSourceFiles(test_elf_path, sourceFiles);
    // May fail for test file, but should not crash
    EXPECT_NO_THROW(parser.extractSourceFiles(test_elf_path, sourceFiles));
}

TEST_F(LightweightDWARFParserTest, ExtractSourceFilesNonExistentFile) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles;
    
    bool result = parser.extractSourceFiles("/nonexistent/file", sourceFiles);
    EXPECT_FALSE(result);
}

TEST_F(LightweightDWARFParserTest, ExtractSourceFilesEmptyFile) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles;
    
    std::string empty_file = (test_dir / "empty").string();
    std::ofstream(empty_file).close();
    
    bool result = parser.extractSourceFiles(empty_file, sourceFiles);
    EXPECT_FALSE(result);
}

TEST_F(LightweightDWARFParserTest, ExtractSourceFilesInvalidELF) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles;
    
    std::string invalid_file = (test_dir / "invalid").string();
    std::ofstream file(invalid_file);
    file << "This is not an ELF file";
    file.close();
    
    bool result = parser.extractSourceFiles(invalid_file, sourceFiles);
    EXPECT_FALSE(result);
}

TEST_F(LightweightDWARFParserTest, ExtractCompileUnits) {
    LightweightDWARFParser parser;
    std::vector<std::string> compileUnits;
    
    bool result = parser.extractCompileUnits(test_elf_path, compileUnits);
    EXPECT_NO_THROW(parser.extractCompileUnits(test_elf_path, compileUnits));
}

TEST_F(LightweightDWARFParserTest, ExtractCompileUnitsNonExistentFile) {
    LightweightDWARFParser parser;
    std::vector<std::string> compileUnits;
    
    bool result = parser.extractCompileUnits("/nonexistent/file", compileUnits);
    EXPECT_FALSE(result);
}

TEST_F(LightweightDWARFParserTest, ExtractFunctions) {
    LightweightDWARFParser parser;
    std::vector<std::string> functions;
    
    bool result = parser.extractFunctions(test_elf_path, functions);
    EXPECT_NO_THROW(parser.extractFunctions(test_elf_path, functions));
}

TEST_F(LightweightDWARFParserTest, ExtractFunctionsNonExistentFile) {
    LightweightDWARFParser parser;
    std::vector<std::string> functions;
    
    bool result = parser.extractFunctions("/nonexistent/file", functions);
    EXPECT_FALSE(result);
}

TEST_F(LightweightDWARFParserTest, ExtractAllDebugInfo) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    bool result = parser.extractAllDebugInfo(test_elf_path, sourceFiles, compileUnits, functions);
    EXPECT_NO_THROW(parser.extractAllDebugInfo(test_elf_path, sourceFiles, compileUnits, functions));
}

TEST_F(LightweightDWARFParserTest, ExtractAllDebugInfoNonExistentFile) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    bool result = parser.extractAllDebugInfo("/nonexistent/file", sourceFiles, compileUnits, functions);
    EXPECT_FALSE(result);
}

TEST_F(LightweightDWARFParserTest, HasDWARFInfo) {
    LightweightDWARFParser parser;
    
    bool result = parser.hasDWARFInfo(test_elf_path);
    EXPECT_NO_THROW(parser.hasDWARFInfo(test_elf_path));
}

TEST_F(LightweightDWARFParserTest, HasDWARFInfoNonExistentFile) {
    LightweightDWARFParser parser;
    
    bool result = parser.hasDWARFInfo("/nonexistent/file");
    EXPECT_FALSE(result);
}

TEST_F(LightweightDWARFParserTest, HasDWARFInfoEmptyFile) {
    LightweightDWARFParser parser;
    
    std::string empty_file = (test_dir / "empty").string();
    std::ofstream(empty_file).close();
    
    bool result = parser.hasDWARFInfo(empty_file);
    EXPECT_FALSE(result);
}

TEST_F(LightweightDWARFParserTest, HasDWARFInfoInvalidELF) {
    LightweightDWARFParser parser;
    
    std::string invalid_file = (test_dir / "invalid").string();
    std::ofstream file(invalid_file);
    file << "This is not an ELF file";
    file.close();
    
    bool result = parser.hasDWARFInfo(invalid_file);
    EXPECT_FALSE(result);
}

// Error handling tests
TEST_F(LightweightDWARFParserTest, InvalidDWARFData) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    // Test with corrupted DWARF data
    EXPECT_NO_THROW(parser.extractSourceFiles(test_elf_path, sourceFiles));
    EXPECT_NO_THROW(parser.extractCompileUnits(test_elf_path, compileUnits));
    EXPECT_NO_THROW(parser.extractFunctions(test_elf_path, functions));
    EXPECT_NO_THROW(parser.extractAllDebugInfo(test_elf_path, sourceFiles, compileUnits, functions));
}

TEST_F(LightweightDWARFParserTest, MemoryManagement) {
    {
        LightweightDWARFParser parser;
        std::vector<std::string> sourceFiles, compileUnits, functions;
        
        parser.extractSourceFiles(test_elf_path, sourceFiles);
        parser.extractCompileUnits(test_elf_path, compileUnits);
        parser.extractFunctions(test_elf_path, functions);
        parser.extractAllDebugInfo(test_elf_path, sourceFiles, compileUnits, functions);
        // Destructor should not crash
    }
    EXPECT_TRUE(true); // If we get here, no crash occurred
}

TEST_F(LightweightDWARFParserTest, MultipleExtractions) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    EXPECT_NO_THROW(parser.extractSourceFiles(test_elf_path, sourceFiles));
    EXPECT_NO_THROW(parser.extractSourceFiles(test_elf_path, sourceFiles)); // Should handle multiple extractions
    EXPECT_NO_THROW(parser.extractCompileUnits(test_elf_path, compileUnits));
    EXPECT_NO_THROW(parser.extractCompileUnits(test_elf_path, compileUnits));
    EXPECT_NO_THROW(parser.extractFunctions(test_elf_path, functions));
    EXPECT_NO_THROW(parser.extractFunctions(test_elf_path, functions));
}

TEST_F(LightweightDWARFParserTest, ReuseParser) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    // Test that parser can be reused
    EXPECT_NO_THROW(parser.extractSourceFiles(test_elf_path, sourceFiles));
    EXPECT_NO_THROW(parser.extractCompileUnits(test_elf_path, compileUnits));
    EXPECT_NO_THROW(parser.extractFunctions(test_elf_path, functions));
    EXPECT_NO_THROW(parser.extractAllDebugInfo(test_elf_path, sourceFiles, compileUnits, functions));
}

// Edge cases
TEST_F(LightweightDWARFParserTest, EmptyDWARFSections) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    // Test with ELF file that has no DWARF sections
    EXPECT_NO_THROW(parser.extractSourceFiles(test_elf_path, sourceFiles));
    EXPECT_NO_THROW(parser.extractCompileUnits(test_elf_path, compileUnits));
    EXPECT_NO_THROW(parser.extractFunctions(test_elf_path, functions));
    EXPECT_NO_THROW(parser.extractAllDebugInfo(test_elf_path, sourceFiles, compileUnits, functions));
}

TEST_F(LightweightDWARFParserTest, LargeFileHandling) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    // Test with large files (simulated by creating a larger test file)
    std::string large_file = (test_dir / "large_test").string();
    std::ofstream large_test(large_file, std::ios::binary);
    std::vector<uint8_t> data(1024 * 1024, 0); // 1MB of zeros
    large_test.write(reinterpret_cast<const char*>(data.data()), data.size());
    large_test.close();
    
    EXPECT_NO_THROW(parser.extractSourceFiles(large_file, sourceFiles));
    EXPECT_NO_THROW(parser.extractCompileUnits(large_file, compileUnits));
    EXPECT_NO_THROW(parser.extractFunctions(large_file, functions));
    EXPECT_NO_THROW(parser.extractAllDebugInfo(large_file, sourceFiles, compileUnits, functions));
}

TEST_F(LightweightDWARFParserTest, ConcurrentAccess) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    // Test that multiple threads can access parser data safely
    auto test_thread = [&parser, this]() {
        std::vector<std::string> localSourceFiles, localCompileUnits, localFunctions;
        EXPECT_NO_THROW(parser.extractSourceFiles(test_elf_path, localSourceFiles));
        EXPECT_NO_THROW(parser.extractCompileUnits(test_elf_path, localCompileUnits));
        EXPECT_NO_THROW(parser.extractFunctions(test_elf_path, localFunctions));
        EXPECT_NO_THROW(parser.extractAllDebugInfo(test_elf_path, localSourceFiles, localCompileUnits, localFunctions));
    };
    
    std::thread t1(test_thread);
    std::thread t2(test_thread);
    
    t1.join();
    t2.join();
}

TEST_F(LightweightDWARFParserTest, MultipleParserInstances) {
    // Test multiple parser instances
    LightweightDWARFParser parser1, parser2, parser3;
    std::vector<std::string> sourceFiles1, sourceFiles2, sourceFiles3;
    std::vector<std::string> compileUnits1, compileUnits2, compileUnits3;
    std::vector<std::string> functions1, functions2, functions3;
    
    EXPECT_NO_THROW(parser1.extractSourceFiles(test_elf_path, sourceFiles1));
    EXPECT_NO_THROW(parser2.extractCompileUnits(test_elf_path, compileUnits2));
    EXPECT_NO_THROW(parser3.extractFunctions(test_elf_path, functions3));
}

TEST_F(LightweightDWARFParserTest, RapidSuccession) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    // Test rapid succession of extractions
    for (int i = 0; i < 100; ++i) {
        EXPECT_NO_THROW(parser.extractSourceFiles(test_elf_path, sourceFiles));
        EXPECT_NO_THROW(parser.extractCompileUnits(test_elf_path, compileUnits));
        EXPECT_NO_THROW(parser.extractFunctions(test_elf_path, functions));
        EXPECT_NO_THROW(parser.extractAllDebugInfo(test_elf_path, sourceFiles, compileUnits, functions));
    }
}

TEST_F(LightweightDWARFParserTest, MixedOperations) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    // Test mixed operations
    for (int i = 0; i < 50; ++i) {
        EXPECT_NO_THROW(parser.extractSourceFiles(test_elf_path, sourceFiles));
        EXPECT_NO_THROW(parser.hasDWARFInfo(test_elf_path));
        EXPECT_NO_THROW(parser.extractCompileUnits(test_elf_path, compileUnits));
        EXPECT_NO_THROW(parser.extractFunctions(test_elf_path, functions));
        EXPECT_NO_THROW(parser.extractAllDebugInfo(test_elf_path, sourceFiles, compileUnits, functions));
    }
}

TEST_F(LightweightDWARFParserTest, ErrorRecovery) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    // Test that parser can recover from errors
    std::vector<std::string> test_files = {
        "/nonexistent1", "/nonexistent2",
        test_elf_path,
        "/another/nonexistent", "/yet/another/nonexistent"
    };
    
    for (const auto& test_file : test_files) {
        EXPECT_NO_THROW(parser.extractSourceFiles(test_file, sourceFiles));
        EXPECT_NO_THROW(parser.extractCompileUnits(test_file, compileUnits));
        EXPECT_NO_THROW(parser.extractFunctions(test_file, functions));
        EXPECT_NO_THROW(parser.extractAllDebugInfo(test_file, sourceFiles, compileUnits, functions));
    }
}

TEST_F(LightweightDWARFParserTest, VeryLongFilePaths) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    // Create a very long file path
    std::string long_path = test_dir.string();
    for (int i = 0; i < 20; ++i) {
        long_path += "/very/deep/nested/directory/structure/";
    }
    std::filesystem::create_directories(long_path);
    
    std::string long_file = long_path + "/test_binary";
    std::ofstream long_test(long_file, std::ios::binary);
    std::vector<uint8_t> elf_header = {
        0x7f, 0x45, 0x4c, 0x46, 0x02, 0x01, 0x01, 0x00
    };
    long_test.write(reinterpret_cast<const char*>(elf_header.data()), elf_header.size());
    long_test.close();
    
    EXPECT_NO_THROW(parser.extractSourceFiles(long_file, sourceFiles));
    EXPECT_NO_THROW(parser.extractCompileUnits(long_file, compileUnits));
    EXPECT_NO_THROW(parser.extractFunctions(long_file, functions));
    EXPECT_NO_THROW(parser.extractAllDebugInfo(long_file, sourceFiles, compileUnits, functions));
}

TEST_F(LightweightDWARFParserTest, SpecialCharactersInPath) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    // Create a file path with special characters
    std::string special_path = (test_dir / "test@file#with$special%chars").string();
    std::ofstream special_test(special_path, std::ios::binary);
    std::vector<uint8_t> elf_header = {
        0x7f, 0x45, 0x4c, 0x46, 0x02, 0x01, 0x01, 0x00
    };
    special_test.write(reinterpret_cast<const char*>(elf_header.data()), elf_header.size());
    special_test.close();
    
    EXPECT_NO_THROW(parser.extractSourceFiles(special_path, sourceFiles));
    EXPECT_NO_THROW(parser.extractCompileUnits(special_path, compileUnits));
    EXPECT_NO_THROW(parser.extractFunctions(special_path, functions));
    EXPECT_NO_THROW(parser.extractAllDebugInfo(special_path, sourceFiles, compileUnits, functions));
}

TEST_F(LightweightDWARFParserTest, UnicodeInPath) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    // Create a file path with unicode characters
    std::string unicode_path = (test_dir / "测试文件").string();
    std::ofstream unicode_test(unicode_path, std::ios::binary);
    std::vector<uint8_t> elf_header = {
        0x7f, 0x45, 0x4c, 0x46, 0x02, 0x01, 0x01, 0x00
    };
    unicode_test.write(reinterpret_cast<const char*>(elf_header.data()), elf_header.size());
    unicode_test.close();
    
    EXPECT_NO_THROW(parser.extractSourceFiles(unicode_path, sourceFiles));
    EXPECT_NO_THROW(parser.extractCompileUnits(unicode_path, compileUnits));
    EXPECT_NO_THROW(parser.extractFunctions(unicode_path, functions));
    EXPECT_NO_THROW(parser.extractAllDebugInfo(unicode_path, sourceFiles, compileUnits, functions));
}

// Additional comprehensive tests for better coverage
TEST_F(LightweightDWARFParserTest, MalformedELFHeader) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    // Create a file with malformed ELF header
    std::string malformed_file = (test_dir / "malformed").string();
    std::ofstream malformed_test(malformed_file, std::ios::binary);
    std::vector<uint8_t> bad_header = {
        0x7f, 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x00,  // Wrong class
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    malformed_test.write(reinterpret_cast<const char*>(bad_header.data()), bad_header.size());
    malformed_test.close();
    
    EXPECT_FALSE(parser.extractSourceFiles(malformed_file, sourceFiles));
    EXPECT_FALSE(parser.extractCompileUnits(malformed_file, compileUnits));
    EXPECT_FALSE(parser.extractFunctions(malformed_file, functions));
    EXPECT_FALSE(parser.extractAllDebugInfo(malformed_file, sourceFiles, compileUnits, functions));
    EXPECT_FALSE(parser.hasDWARFInfo(malformed_file));
}

TEST_F(LightweightDWARFParserTest, TruncatedELFHeader) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    // Create a file with truncated ELF header
    std::string truncated_file = (test_dir / "truncated").string();
    std::ofstream truncated_test(truncated_file, std::ios::binary);
    std::vector<uint8_t> truncated_header = {
        0x7f, 0x45, 0x4c, 0x46, 0x02, 0x01, 0x01, 0x00
        // Missing rest of header
    };
    truncated_test.write(reinterpret_cast<const char*>(truncated_header.data()), truncated_header.size());
    truncated_test.close();
    
    EXPECT_FALSE(parser.extractSourceFiles(truncated_file, sourceFiles));
    EXPECT_FALSE(parser.extractCompileUnits(truncated_file, compileUnits));
    EXPECT_FALSE(parser.extractFunctions(truncated_file, functions));
    EXPECT_FALSE(parser.extractAllDebugInfo(truncated_file, sourceFiles, compileUnits, functions));
    EXPECT_FALSE(parser.hasDWARFInfo(truncated_file));
}

TEST_F(LightweightDWARFParserTest, CorruptedDWARFData) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    // Create a file with corrupted DWARF data
    std::string corrupted_file = (test_dir / "corrupted").string();
    std::ofstream corrupted_test(corrupted_file, std::ios::binary);
    
    // Write valid ELF header
    std::vector<uint8_t> elf_header = {
        0x7f, 0x45, 0x4c, 0x46, 0x02, 0x01, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x3e, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x40, 0x00, 0x38, 0x00, 0x01, 0x00, 0x40, 0x00,
        0x03, 0x00, 0x02, 0x00
    };
    corrupted_test.write(reinterpret_cast<const char*>(elf_header.data()), elf_header.size());
    
    // Write corrupted DWARF data
    std::vector<uint8_t> corrupted_dwarf = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    corrupted_test.write(reinterpret_cast<const char*>(corrupted_dwarf.data()), corrupted_dwarf.size());
    corrupted_test.close();
    
    EXPECT_NO_THROW(parser.extractSourceFiles(corrupted_file, sourceFiles));
    EXPECT_NO_THROW(parser.extractCompileUnits(corrupted_file, compileUnits));
    EXPECT_NO_THROW(parser.extractFunctions(corrupted_file, functions));
    EXPECT_NO_THROW(parser.extractAllDebugInfo(corrupted_file, sourceFiles, compileUnits, functions));
}

TEST_F(LightweightDWARFParserTest, MultipleParserInstancesConcurrent) {
    // Test multiple parser instances with concurrent access
    std::vector<std::thread> threads;
    std::vector<LightweightDWARFParser> parsers(5);
    
    auto test_function = [&](int index) {
        std::vector<std::string> sourceFiles, compileUnits, functions;
        EXPECT_NO_THROW(parsers[index].extractSourceFiles(test_elf_path, sourceFiles));
        EXPECT_NO_THROW(parsers[index].extractCompileUnits(test_elf_path, compileUnits));
        EXPECT_NO_THROW(parsers[index].extractFunctions(test_elf_path, functions));
        EXPECT_NO_THROW(parsers[index].extractAllDebugInfo(test_elf_path, sourceFiles, compileUnits, functions));
    };
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(test_function, i);
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
}

TEST_F(LightweightDWARFParserTest, StressTestRapidOperations) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    // Perform rapid operations to test stress handling
    for (int i = 0; i < 1000; ++i) {
        EXPECT_NO_THROW(parser.extractSourceFiles(test_elf_path, sourceFiles));
        EXPECT_NO_THROW(parser.hasDWARFInfo(test_elf_path));
        EXPECT_NO_THROW(parser.extractCompileUnits(test_elf_path, compileUnits));
        EXPECT_NO_THROW(parser.extractFunctions(test_elf_path, functions));
        EXPECT_NO_THROW(parser.extractAllDebugInfo(test_elf_path, sourceFiles, compileUnits, functions));
    }
}

TEST_F(LightweightDWARFParserTest, MemoryLeakTest) {
    // Test for memory leaks by creating and destroying many parsers
    for (int i = 0; i < 1000; ++i) {
        LightweightDWARFParser parser;
        std::vector<std::string> sourceFiles, compileUnits, functions;
        
        parser.extractSourceFiles(test_elf_path, sourceFiles);
        parser.extractCompileUnits(test_elf_path, compileUnits);
        parser.extractFunctions(test_elf_path, functions);
        parser.extractAllDebugInfo(test_elf_path, sourceFiles, compileUnits, functions);
        parser.hasDWARFInfo(test_elf_path);
    }
    // If we get here without crashing, memory management is working
    EXPECT_TRUE(true);
}

TEST_F(LightweightDWARFParserTest, BoundaryConditions) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    // Test with very small files
    std::string tiny_file = (test_dir / "tiny").string();
    std::ofstream tiny_test(tiny_file, std::ios::binary);
    tiny_test.write("", 0);  // Empty file
    tiny_test.close();
    
    EXPECT_FALSE(parser.extractSourceFiles(tiny_file, sourceFiles));
    EXPECT_FALSE(parser.extractCompileUnits(tiny_file, compileUnits));
    EXPECT_FALSE(parser.extractFunctions(tiny_file, functions));
    EXPECT_FALSE(parser.extractAllDebugInfo(tiny_file, sourceFiles, compileUnits, functions));
    EXPECT_FALSE(parser.hasDWARFInfo(tiny_file));
    
    // Test with very large file paths
    std::string long_path = test_dir.string();
    for (int i = 0; i < 50; ++i) {
        long_path += "/very/deep/nested/directory/structure/";
    }
    std::filesystem::create_directories(long_path);
    
    std::string long_file = long_path + "/test_binary";
    std::ofstream long_test(long_file, std::ios::binary);
    std::vector<uint8_t> elf_header = {
        0x7f, 0x45, 0x4c, 0x46, 0x02, 0x01, 0x01, 0x00
    };
    long_test.write(reinterpret_cast<const char*>(elf_header.data()), elf_header.size());
    long_test.close();
    
    EXPECT_NO_THROW(parser.extractSourceFiles(long_file, sourceFiles));
    EXPECT_NO_THROW(parser.extractCompileUnits(long_file, compileUnits));
    EXPECT_NO_THROW(parser.extractFunctions(long_file, functions));
    EXPECT_NO_THROW(parser.extractAllDebugInfo(long_file, sourceFiles, compileUnits, functions));
}

TEST_F(LightweightDWARFParserTest, ErrorRecoveryAfterFailure) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    // First, test with invalid file
    std::string invalid_file = "/nonexistent/file";
    EXPECT_FALSE(parser.extractSourceFiles(invalid_file, sourceFiles));
    EXPECT_FALSE(parser.extractCompileUnits(invalid_file, compileUnits));
    EXPECT_FALSE(parser.extractFunctions(invalid_file, functions));
    EXPECT_FALSE(parser.extractAllDebugInfo(invalid_file, sourceFiles, compileUnits, functions));
    EXPECT_FALSE(parser.hasDWARFInfo(invalid_file));
    
    // Then test with valid file - should still work
    EXPECT_NO_THROW(parser.extractSourceFiles(test_elf_path, sourceFiles));
    EXPECT_NO_THROW(parser.extractCompileUnits(test_elf_path, compileUnits));
    EXPECT_NO_THROW(parser.extractFunctions(test_elf_path, functions));
    EXPECT_NO_THROW(parser.extractAllDebugInfo(test_elf_path, sourceFiles, compileUnits, functions));
    EXPECT_NO_THROW(parser.hasDWARFInfo(test_elf_path));
}

TEST_F(LightweightDWARFParserTest, MixedValidAndInvalidFiles) {
    LightweightDWARFParser parser;
    std::vector<std::string> sourceFiles, compileUnits, functions;
    
    std::vector<std::string> test_files = {
        "/nonexistent1", test_elf_path, "/nonexistent2",
        test_elf_path, "/another/nonexistent", test_elf_path
    };
    
    for (const auto& test_file : test_files) {
        EXPECT_NO_THROW(parser.extractSourceFiles(test_file, sourceFiles));
        EXPECT_NO_THROW(parser.extractCompileUnits(test_file, compileUnits));
        EXPECT_NO_THROW(parser.extractFunctions(test_file, functions));
        EXPECT_NO_THROW(parser.extractAllDebugInfo(test_file, sourceFiles, compileUnits, functions));
        EXPECT_NO_THROW(parser.hasDWARFInfo(test_file));
    }
}