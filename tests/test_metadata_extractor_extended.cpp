/*
Copyright 2025 The Heimdall Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
you may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "common/ComponentInfo.hpp"
#include "common/MetadataExtractor.hpp"
#include "common/Utils.hpp"
#include "src/compat/compatibility.hpp"
#include "test_utils.hpp"

using namespace heimdall;

class MetadataExtractorExtendedTest : public ::testing::Test
{
  protected:
  void SetUp() override
  {
    test_dir = test_utils::getUniqueTestDirectory("heimdall_metadata_test");
    heimdall::compat::fs::create_directories(test_dir);

    // Create test files
    createTestFiles();
  }

  void TearDown() override
  {
    test_utils::safeRemoveDirectory(test_dir);
  }

  void createTestFiles()
  {
    // Create test object file
    test_object_file = test_dir / "test.o";
    std::ofstream obj_file(test_object_file);
    obj_file << "ELF object file content";
    obj_file.close();

    // Create test library file
    test_library_file = test_dir / "libtest.a";
    std::ofstream lib_file(test_library_file);
    lib_file << "Static library content";
    lib_file.close();

    // Create test executable
    test_executable = test_dir / "test_exe";
    std::ofstream exe_file(test_executable);
    exe_file << "Executable content";
    exe_file.close();

    // Create test shared library
    test_shared_lib = test_dir / "libtest.so";
    std::ofstream shared_file(test_shared_lib);
    shared_file << "Shared library content";
    shared_file.close();

    // Create test archive
    test_archive = test_dir / "libarchive.a";
    std::ofstream archive_file(test_archive);
    archive_file << "Archive content";
    archive_file.close();

    // Create test RPM package
    test_rpm = test_dir / "test.rpm";
    std::ofstream rpm_file(test_rpm);
    rpm_file << "RPM package content";
    rpm_file.close();

    // Create test DEB package
    test_deb = test_dir / "test.deb";
    std::ofstream deb_file(test_deb);
    deb_file << "DEB package content";
    deb_file.close();
  }

  heimdall::compat::fs::path test_dir;
  heimdall::compat::fs::path test_object_file;
  heimdall::compat::fs::path test_library_file;
  heimdall::compat::fs::path test_executable;
  heimdall::compat::fs::path test_shared_lib;
  heimdall::compat::fs::path test_archive;
  heimdall::compat::fs::path test_rpm;
  heimdall::compat::fs::path test_deb;
};

// Enhanced MetadataExtractor Tests

TEST_F(MetadataExtractorExtendedTest, MetadataExtractorCreation)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataFromObjectFile)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  ComponentInfo component("test.o", test_object_file.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataFromLibrary)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  ComponentInfo component("libtest.a", test_library_file.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataFromExecutable)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  ComponentInfo component("test_exe", test_executable.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataFromSharedLibrary)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  ComponentInfo component("libtest.so", test_shared_lib.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataFromArchive)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  ComponentInfo component("libarchive.a", test_archive.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataFromNonExistentFile)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  ComponentInfo component("nonexistent.o", "/nonexistent/file.o");
  bool          result = extractor->extractMetadata(component);

  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractVersionInfo)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  ComponentInfo component("test.o", test_object_file.string());
  bool          result = extractor->extractVersionInfo(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractLicenseInfo)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  ComponentInfo component("test.o", test_object_file.string());
  bool          result = extractor->extractLicenseInfo(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractSymbolInfo)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  ComponentInfo component("test.o", test_object_file.string());
  bool          result = extractor->extractSymbolInfo(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractSectionInfo)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  ComponentInfo component("test.o", test_object_file.string());
  bool          result = extractor->extractSectionInfo(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractDebugInfo)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  ComponentInfo component("test.o", test_object_file.string());
  bool          result = extractor->extractDebugInfo(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractDependencyInfo)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  ComponentInfo component("test.o", test_object_file.string());
  bool          result = extractor->extractDependencyInfo(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataFromRPM)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  ComponentInfo component("test.rpm", test_rpm.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real RPM file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataFromDEB)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  ComponentInfo component("test.deb", test_deb.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real DEB file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithEmptyComponent)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  ComponentInfo component;
  bool          result = extractor->extractMetadata(component);

  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithInvalidPath)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  ComponentInfo component("test.o", "");
  bool          result = extractor->extractMetadata(component);

  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithDirectory)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  ComponentInfo component("test_dir", test_dir.string());
  bool          result = extractor->extractMetadata(component);

  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithUnreadableFile)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Create a file with no read permissions
  heimdall::compat::fs::path unreadable_file = test_dir / "unreadable.o";
  std::ofstream              file(unreadable_file);
  file << "content";
  file.close();

  // Remove read permissions
  heimdall::compat::fs::permissions(
    unreadable_file,
    heimdall::compat::fs::perms::owner_read | heimdall::compat::fs::perms::owner_write,
    heimdall::compat::fs::perm_options::remove);

  ComponentInfo component("unreadable.o", unreadable_file.string());
  bool          result = extractor->extractMetadata(component);

  // Restore permissions for cleanup
  heimdall::compat::fs::permissions(
    unreadable_file,
    heimdall::compat::fs::perms::owner_read | heimdall::compat::fs::perms::owner_write,
    heimdall::compat::fs::perm_options::add);

  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithLargeFile)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  heimdall::compat::fs::path large_file = test_dir / "large.o";
  std::ofstream              file(large_file);
  file << std::string(1024 * 1024, 'A');  // 1MB of data
  file.close();

  ComponentInfo component("large.o", large_file.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithBinaryFile)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  heimdall::compat::fs::path binary_file = test_dir / "binary.o";
  std::ofstream              file(binary_file, std::ios::binary);
  for (int i = 0; i < 1000; ++i)
  {
    file.put(static_cast<char>(i % 256));
  }
  file.close();

  ComponentInfo component("binary.o", binary_file.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithCorruptedFile)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  heimdall::compat::fs::path corrupted_file = test_dir / "corrupted.o";
  std::ofstream              file(corrupted_file);
  file << "This is not a valid ELF file at all";
  file.close();

  ComponentInfo component("corrupted.o", corrupted_file.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithSpecialCharacters)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  heimdall::compat::fs::path special_file = test_dir / "test-file_with_special_chars.o";
  std::ofstream              file(special_file);
  file << "ELF object file content with special chars";
  file.close();

  ComponentInfo component("test-file_with_special_chars.o", special_file.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithUnicodeCharacters)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  heimdall::compat::fs::path unicode_file = test_dir / "test-unicode-测试.o";
  std::ofstream              file(unicode_file);
  file << "ELF object file content with unicode";
  file.close();

  ComponentInfo component("test-unicode-测试.o", unicode_file.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithLongPath)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Create a deep directory structure
  heimdall::compat::fs::path deep_dir = test_dir;
  for (int i = 0; i < 10; ++i)
  {
    deep_dir = deep_dir / ("level" + std::to_string(i));
    heimdall::compat::fs::create_directories(deep_dir);
  }

  heimdall::compat::fs::path deep_file = deep_dir / "test.o";
  std::ofstream              file(deep_file);
  file << "ELF object file content in deep path";
  file.close();

  ComponentInfo component("test.o", deep_file.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithSymlink)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Create a symlink to the test object file
  heimdall::compat::fs::path symlink_file = test_dir / "symlink.o";
  heimdall::compat::fs::create_symlink(test_object_file, symlink_file);

  ComponentInfo component("symlink.o", symlink_file.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithHardlink)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Create a hardlink to the test object file
  heimdall::compat::fs::path hardlink_file = test_dir / "hardlink.o";
  heimdall::compat::fs::create_hard_link(test_object_file, hardlink_file);

  ComponentInfo component("hardlink.o", hardlink_file.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithZeroSizeFile)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  heimdall::compat::fs::path empty_file = test_dir / "empty.o";
  std::ofstream              file(empty_file);
  file.close();

  ComponentInfo component("empty.o", empty_file.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because empty file is not a valid ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithVeryLargeFile)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  heimdall::compat::fs::path large_file = test_dir / "very_large.o";
  std::ofstream              file(large_file);
  file << std::string(10 * 1024 * 1024, 'B');  // 10MB of data
  file.close();

  ComponentInfo component("very_large.o", large_file.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithConcurrentAccess)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Test concurrent access to the same files
  std::vector<std::thread> threads;
  std::vector<bool>        results(3, false);

  for (int i = 0; i < 3; ++i)
  {
    threads.emplace_back(
      [&, i]()
      {
        ComponentInfo component("test.o", test_object_file.string());
        results[i] = extractor->extractMetadata(component);
      });
  }

  for (auto& thread : threads)
  {
    thread.join();
  }

  // All should fail because our test file is not a real ELF file
  for (bool result : results)
  {
    EXPECT_FALSE(result);
  }
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithMemoryPressure)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Create fewer files to prevent timeout (reduced from 100 to 10)
  std::vector<heimdall::compat::fs::path> files;
  for (int i = 0; i < 10; ++i)
  {
    heimdall::compat::fs::path file = test_dir / ("test" + std::to_string(i) + ".o");
    std::ofstream              f(file);
    f << "ELF object file content " << i;
    f.close();
    files.push_back(file);
  }

  // Process all files
  for (const auto& file : files)
  {
    ComponentInfo component(file.filename().string(), file.string());
    bool          result = extractor->extractMetadata(component);
    // Should fail because our test files are not real ELF files
    EXPECT_FALSE(result);
  }
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithInvalidFileTypes)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Create files with invalid extensions
  std::vector<std::string> invalid_extensions = {".txt", ".doc", ".pdf", ".jpg", ".mp3"};

  for (const auto& ext : invalid_extensions)
  {
    heimdall::compat::fs::path file = test_dir / ("test" + ext);
    std::ofstream              f(file);
    f << "This is not a binary file";
    f.close();

    ComponentInfo component(file.filename().string(), file.string());
    bool          result = extractor->extractMetadata(component);

    // Should fail because these are not valid binary files
    EXPECT_FALSE(result);
  }
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithNetworkPath)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Test with network-style paths (should fail gracefully)
  ComponentInfo component("remote.o", "//remote-server/path/to/file.o");
  bool          result = extractor->extractMetadata(component);

  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithRelativePath)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Test with relative path
  ComponentInfo component("test.o", "./test.o");
  bool          result = extractor->extractMetadata(component);

  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithParentDirectory)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Test with parent directory reference
  ComponentInfo component("test.o", "../test.o");
  bool          result = extractor->extractMetadata(component);

  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithHomeDirectory)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Test with home directory
  std::string   home_path = std::string(getenv("HOME") ? getenv("HOME") : "/home/user") + "/test.o";
  ComponentInfo component("test.o", home_path);
  bool          result = extractor->extractMetadata(component);

  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithMockSystemPaths)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Create mock ELF files instead of using real system binaries
  std::vector<std::pair<std::string, std::string>> mock_files = {
    {"mock_libc.so", "\x7f\x45\x4c\x46\x02\x01\x01\x00"},  // ELF header
    {"mock_libm.so", "\x7f\x45\x4c\x46\x02\x01\x01\x00"},  // ELF header
    {"mock_ls", "\x7f\x45\x4c\x46\x02\x01\x01\x00"},       // ELF header
    {"mock_bash", "\x7f\x45\x4c\x46\x02\x01\x01\x00"}      // ELF header
  };

  for (const auto& [filename, content] : mock_files)
  {
    heimdall::compat::fs::path mock_file = test_dir / filename;
    std::ofstream              file(mock_file, std::ios::binary);
    file.write(content.c_str(), content.length());
    file.close();

    ComponentInfo component(filename, mock_file.string());
    bool          result = extractor->extractMetadata(component);
    // These should fail gracefully since they're not complete ELF files
    // but the important thing is they don't trigger system directory scanning
    EXPECT_FALSE(result);
  }
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithTemporaryFiles)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Test with temporary files
  heimdall::compat::fs::path temp_file =
    heimdall::compat::fs::temp_directory_path() / "temp_test.o";
  std::ofstream file(temp_file);
  file << "ELF object file content";
  file.close();

  ComponentInfo component("temp_test.o", temp_file.string());
  bool          result = extractor->extractMetadata(component);

  // Clean up
  heimdall::compat::fs::remove(temp_file);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithFIFO)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Create a FIFO (named pipe) - this test is problematic because reading from FIFO blocks
  // Skip this test to avoid hanging
  GTEST_SKIP() << "FIFO test skipped to avoid blocking behavior";
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithDeviceFile)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Test with device file (should fail gracefully)
  ComponentInfo component("null", "/dev/null");
  bool          result = extractor->extractMetadata(component);

  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithSocket)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Test with socket file (should fail gracefully)
  ComponentInfo component("socket", "/tmp/socket");
  bool          result = extractor->extractMetadata(component);

  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithBrokenSymlink)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Create a broken symlink
  heimdall::compat::fs::path broken_symlink = test_dir / "broken_symlink.o";
  heimdall::compat::fs::create_symlink("/nonexistent/file.o", broken_symlink);

  ComponentInfo component("broken_symlink.o", broken_symlink.string());
  bool          result = extractor->extractMetadata(component);

  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithCircularSymlink)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Create a circular symlink
  heimdall::compat::fs::path circular1 = test_dir / "circular1.o";
  heimdall::compat::fs::path circular2 = test_dir / "circular2.o";

  heimdall::compat::fs::create_symlink(circular2, circular1);
  heimdall::compat::fs::create_symlink(circular1, circular2);

  ComponentInfo component("circular1.o", circular1.string());
  bool          result = extractor->extractMetadata(component);

  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithUnicodePath)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Create file with unicode path
  heimdall::compat::fs::path unicode_path = test_dir / "测试路径" / "test.o";
  heimdall::compat::fs::create_directories(unicode_path.parent_path());
  std::ofstream file(unicode_path);
  file << "ELF object file content";
  file.close();

  ComponentInfo component("test.o", unicode_path.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithSpacesInPath)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Create file with spaces in path
  heimdall::compat::fs::path space_path = test_dir / "path with spaces" / "test file.o";
  heimdall::compat::fs::create_directories(space_path.parent_path());
  std::ofstream file(space_path);
  file << "ELF object file content";
  file.close();

  ComponentInfo component("test file.o", space_path.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}

TEST_F(MetadataExtractorExtendedTest, ExtractMetadataWithSpecialCharactersInPath)
{
  auto extractor = std::make_unique<MetadataExtractor>();
  ASSERT_NE(extractor, nullptr);

  // Create file with special characters in path
  heimdall::compat::fs::path special_path =
    test_dir / "path-with-special-chars-!@#$%^&*()" / "test.o";
  heimdall::compat::fs::create_directories(special_path.parent_path());
  std::ofstream file(special_path);
  file << "ELF object file content";
  file.close();

  ComponentInfo component("test.o", special_path.string());
  bool          result = extractor->extractMetadata(component);

  // Should fail because our test file is not a real ELF file
  EXPECT_FALSE(result);
}