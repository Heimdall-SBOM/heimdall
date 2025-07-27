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

/**
 * @file test_plugin_interface.hpp
 * @brief Test interface declarations for plugin functions
 * @author Trevor Bakker
 * @date 2025
 */

#pragma once

#include <cstdint>

#ifdef __cplusplus
extern "C"
{
#endif

   // Gold Plugin C-style function declarations
   int         onload(void* handle);
   void        onunload(void);
   const char* heimdall_gold_version(void);
   const char* heimdall_gold_description(void);
   int         heimdall_set_output_path(const char* path);
   int         heimdall_set_format(const char* fmt);
   void        heimdall_set_verbose(bool v);
   int         heimdall_process_input_file(const char* filePath);
   int         heimdall_process_library(const char* libraryPath);
   int         heimdall_process_symbol(const char* symbolName, uint64_t address, uint64_t size);
   int         heimdall_set_cyclonedx_version(const char* version);
   void        heimdall_finalize(void);
   int         heimdall_gold_set_plugin_option(const char* option);

   // LLD Plugin C-style function declarations
   const char* heimdall_lld_version(void);
   const char* heimdall_lld_description(void);

   int         heimdall_lld_set_plugin_option(const char* option);

#ifdef __cplusplus
}
#endif