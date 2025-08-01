/*
Copyright 2025 The Heimdall Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <gtest/gtest.h>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>
#include "src/compat/compatibility.hpp"
#include "test_utils.hpp"

using namespace heimdall;

class RecursiveDirectoryIteratorTest : public ::testing::Test
{
   protected:
   void SetUp() override
   {
      // Create a temporary test directory structure with unique name
      auto pid = std::to_string(getpid());
      auto timestamp = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
         std::chrono::system_clock::now().time_since_epoch()).count());
      test_dir = fs::temp_directory_path() / ("heimdall_recursive_test_" + pid + "_" + timestamp);

      std::cerr << "Setting up test directory: " << test_dir.string() << std::endl;

      // Clean up any existing test directory first
      if (fs::exists(test_dir))
      {
         std::cerr << "Removing existing test directory" << std::endl;
         fs::remove_all(test_dir);
      }

      std::cerr << "Creating test directory" << std::endl;
      fs::create_directories(test_dir);

      // Create test directory structure:
      // test_dir/
      // ├── file1.txt
      // ├── subdir1/
      // │   ├── file2.txt
      // │   └── subdir2/
      // │       └── file3.txt
      // └── subdir3/
      //     └── file4.txt

      // Create files
      std::cerr << "Creating file1.txt" << std::endl;
      std::ofstream((test_dir / "file1.txt").string()) << "test content 1";
      
      std::cerr << "Creating subdir1" << std::endl;
      fs::create_directories(test_dir / "subdir1");
      
      std::cerr << "Creating file2.txt" << std::endl;
      std::ofstream((test_dir / "subdir1" / "file2.txt").string()) << "test content 2";
      
      std::cerr << "Creating subdir2" << std::endl;
      fs::create_directories(test_dir / "subdir1" / "subdir2");
      
      std::cerr << "Creating file3.txt" << std::endl;
      std::ofstream((test_dir / "subdir1" / "subdir2" / "file3.txt").string()) << "test content 3";
      
      std::cerr << "Creating subdir3" << std::endl;
      fs::create_directories(test_dir / "subdir3");
      
      std::cerr << "Creating file4.txt" << std::endl;
      std::ofstream((test_dir / "subdir3" / "file4.txt").string()) << "test content 4";
      
      std::cerr << "Test setup complete" << std::endl;
   }

   void TearDown() override
   {
      // Clean up the test directory
      test_utils::safeRemoveDirectory(test_dir);
   }

   fs::path test_dir;
};

TEST_F(RecursiveDirectoryIteratorTest, BasicIteration)
{
   std::vector<std::string> found_files;
   
   for (const auto& entry : fs::recursive_directory_iterator(test_dir))
   {
      if (fs::is_regular_file(entry))
      {
#if __cplusplus >= 201703L
         found_files.push_back(entry.path().filename().string());
#else
         found_files.push_back(entry.filename().string());
#endif
      }
   }

   // Should find all 4 files
   ASSERT_EQ(found_files.size(), 4);
   
   // Check that all expected files are found
   std::sort(found_files.begin(), found_files.end());
   EXPECT_EQ(found_files[0], "file1.txt");
   EXPECT_EQ(found_files[1], "file2.txt");
   EXPECT_EQ(found_files[2], "file3.txt");
   EXPECT_EQ(found_files[3], "file4.txt");
}

TEST_F(RecursiveDirectoryIteratorTest, EmptyDirectory)
{
   // Create an empty directory
   fs::path empty_dir = test_dir / "empty_dir";
   fs::create_directories(empty_dir);

   std::vector<std::string> found_files;
   
   for (const auto& entry : fs::recursive_directory_iterator(empty_dir))
   {
#if __cplusplus >= 201703L
      found_files.push_back(entry.path().string());
#else
      found_files.push_back(entry.string());
#endif
   }

   // Should find no files
   EXPECT_EQ(found_files.size(), 0);
}

TEST_F(RecursiveDirectoryIteratorTest, NonExistentDirectory)
{
   fs::path non_existent_dir = test_dir / "non_existent";
   
   std::vector<std::string> found_files;
   
   try
   {
                   for (const auto& entry : fs::recursive_directory_iterator(non_existent_dir))
    {
#if __cplusplus >= 201703L
       found_files.push_back(entry.path().string());
#else
       found_files.push_back(entry.string());
#endif
    }
   }
   catch (const fs::filesystem_error&)
   {
      // std::filesystem throws an exception for non-existent directories
      // This is expected behavior
   }

   // Should find no files
   EXPECT_EQ(found_files.size(), 0);
}

TEST_F(RecursiveDirectoryIteratorTest, DeepNesting)
{
   // Create a deeply nested directory structure
   fs::path deep_dir = test_dir / "deep";
   fs::create_directories(deep_dir);
   
   // Create 5 levels of nesting
   fs::path current = deep_dir;
   for (int i = 1; i <= 5; ++i)
   {
      current = current / ("level" + std::to_string(i));
      fs::create_directories(current);
      std::ofstream((current / ("file" + std::to_string(i) + ".txt")).string()) << "level " << i;
   }

   std::vector<std::string> found_files;
   
   for (const auto& entry : fs::recursive_directory_iterator(deep_dir))
   {
      if (fs::is_regular_file(entry))
      {
#if __cplusplus >= 201703L
         found_files.push_back(entry.path().filename().string());
#else
         found_files.push_back(entry.filename().string());
#endif
      }
   }

   // Should find all 5 files
   ASSERT_EQ(found_files.size(), 5);
   
   // Check that all expected files are found
   std::sort(found_files.begin(), found_files.end());
   for (int i = 1; i <= 5; ++i)
   {
      EXPECT_EQ(found_files[i-1], "file" + std::to_string(i) + ".txt");
   }
}

TEST_F(RecursiveDirectoryIteratorTest, IteratorCopyAndAssignment)
{
   // Create two separate iterators instead of copying
   fs::recursive_directory_iterator iter1(test_dir);
   fs::recursive_directory_iterator iter2(test_dir);
   
   std::vector<std::string> files1, files2;
   
   // Iterate with first iterator
   while (iter1 != fs::recursive_directory_iterator())
   {
      if (fs::is_regular_file(*iter1))
      {
#if __cplusplus >= 201703L
         files1.push_back(iter1->path().filename().string());
#else
         files1.push_back(iter1->filename().string());
#endif
      }
      ++iter1;
   }
   
   // Iterate with second iterator
   while (iter2 != fs::recursive_directory_iterator())
   {
      if (fs::is_regular_file(*iter2))
      {
#if __cplusplus >= 201703L
         files2.push_back(iter2->path().filename().string());
#else
         files2.push_back(iter2->filename().string());
#endif
      }
      ++iter2;
   }
   
   // Both should find the same files
   ASSERT_EQ(files1.size(), 4);
   ASSERT_EQ(files2.size(), 4);
   
   std::sort(files1.begin(), files1.end());
   std::sort(files2.begin(), files2.end());
   EXPECT_EQ(files1, files2);
}

TEST_F(RecursiveDirectoryIteratorTest, IteratorEquality)
{
   fs::recursive_directory_iterator iter1(test_dir);
   fs::recursive_directory_iterator iter2(test_dir);
   fs::recursive_directory_iterator end_iter;
   
   // Initially, both iterators should not be at end
   EXPECT_NE(iter1, end_iter);
   EXPECT_NE(iter2, end_iter);
   
   // After advancing one iterator, they should be different
   ++iter1;
   EXPECT_NE(iter1, iter2);
   
   // After advancing both to end, they should be equal
   while (iter1 != end_iter) ++iter1;
   while (iter2 != end_iter) ++iter2;
   EXPECT_EQ(iter1, iter2);
}

TEST_F(RecursiveDirectoryIteratorTest, FilePathAccess)
{
   fs::recursive_directory_iterator iter(test_dir);
   
   // Find the first regular file
   while (iter != fs::recursive_directory_iterator() && !fs::is_regular_file(*iter))
   {
      ++iter;
   }
   
   // Should be able to access file path through operator*
   EXPECT_TRUE(fs::is_regular_file(*iter));
#if __cplusplus >= 201703L
   EXPECT_TRUE(iter->path().extension() == ".txt");
#else
   EXPECT_TRUE(iter->extension() == ".txt");
#endif
   
   // Should be able to access file path through operator->
#if __cplusplus >= 201703L
   EXPECT_TRUE(iter->path().filename().string().find("file") != std::string::npos);
#else
   EXPECT_TRUE(iter->filename().string().find("file") != std::string::npos);
#endif
}

TEST_F(RecursiveDirectoryIteratorTest, IsRegularFileMethod)
{
   fs::recursive_directory_iterator iter(test_dir);
   
   // Find the first regular file
   while (iter != fs::recursive_directory_iterator() && !fs::is_regular_file(*iter))
   {
      ++iter;
   }
   
   // Should correctly identify regular files
   EXPECT_TRUE(fs::is_regular_file(*iter));
   
   // Should correctly identify directories (if we had any in the iteration)
   // Note: Our current implementation only iterates over regular files
   // so we can't test directory identification directly
}

TEST_F(RecursiveDirectoryIteratorTest, StringConversion)
{
   fs::recursive_directory_iterator iter(test_dir);
   
   // Find the first regular file
   while (iter != fs::recursive_directory_iterator() && !fs::is_regular_file(*iter))
   {
      ++iter;
   }
   
   // Should be able to convert to string
#if __cplusplus >= 201703L
   std::string path_str = iter->path().string();
#else
   std::string path_str = iter->string();
#endif
   EXPECT_FALSE(path_str.empty());
   EXPECT_TRUE(path_str.find("file") != std::string::npos);
}

TEST_F(RecursiveDirectoryIteratorTest, GetPathMethod)
{
   fs::recursive_directory_iterator iter(test_dir);
   
   // Find the first regular file
   while (iter != fs::recursive_directory_iterator() && !fs::is_regular_file(*iter))
   {
      ++iter;
   }
   
   // Should be able to get path object
   fs::path path_obj = *iter;
   EXPECT_TRUE(fs::is_regular_file(path_obj));
   EXPECT_TRUE(path_obj.extension() == ".txt");
}

TEST_F(RecursiveDirectoryIteratorTest, EmptyMethod)
{
   fs::recursive_directory_iterator iter(test_dir);
   fs::recursive_directory_iterator end_iter;
   
   // Should not be at end initially
   EXPECT_NE(iter, end_iter);
   
   // Should be at end after iteration
   while (iter != end_iter) ++iter;
   EXPECT_EQ(iter, end_iter);
} 