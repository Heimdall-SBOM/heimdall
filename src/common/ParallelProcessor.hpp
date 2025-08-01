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

#pragma once

#include <future>
#include <string>
#include <type_traits>
#include <vector>
#include "../compat/compatibility.hpp"

// Heimdall ParallelProcessor: File-level parallelism for C++17+, serial fallback for C++14.
// DWARF/LLVM debug info extraction must NOT be run in parallel (see README_PROFILING.md).
// Uses heimdall::compat for C++14/17 compatibility.
// Usage:
//   auto results = ParallelProcessor::process(files, [](const std::string& file) { ... });
//   // Each lambda call should be independent and thread-safe.

class ParallelProcessor
{
   public:
   template <typename FileList, typename Func>
   static auto process(const FileList& files, Func&& func)
      -> std::vector<typename std::result_of<Func(const std::string&)>::type>
   {
#if HEIMDALL_CPP17_AVAILABLE
      using Result = typename std::result_of<Func(const std::string&)>::type;

      std::vector<std::future<Result>> futures;
      std::vector<Result>              results;

      results.reserve(files.size());

      for (const auto& file : files)
      {
         futures.push_back(std::async(std::launch::async, func, file));
      }
      for (auto& fut : futures)
      {
         results.push_back(fut.get());
      }

      return results;
#else
      // C++14: Serial fallback
      using Result = typename std::result_of<Func(const std::string&)>::type;

      std::vector<Result> results;

      results.reserve(files.size());

      for (const auto& file : files)
      {
         results.push_back(func(file));
      }

      return results;
#endif
   }
};
