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
 * @file llvm_symbols.cpp
 * @brief LLVM symbols for plugin compatibility
 * @author Trevor Bakker
 * @date 2025
 * 
 * This file provides LLVM symbols that are required for plugin compatibility
 * when linking against LLVM libraries. It ensures that plugins can be loaded
 * without missing symbol errors.
 * 
 * The symbols are exported with default visibility to ensure they are
 * available to the dynamic linker when loading plugins.
 */

// LLVM symbols for plugin compatibility
#include <cstddef>

namespace llvm {
    /**
     * @brief LLVM ABI breaking checks control flag
     * 
     * This symbol is required by some LLVM components and is set to false
     * to disable ABI breaking checks in plugin contexts.
     */
    __attribute__((visibility("default"))) const bool DisableABIBreakingChecks = false;
} 