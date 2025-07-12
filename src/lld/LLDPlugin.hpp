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
 * @file LLDPlugin.hpp
 * @brief LLVM LLD linker plugin interface and implementation
 * @author Trevor Bakker
 * @date 2025
 * 
 * This file provides the interface for the LLVM LLD linker plugin, which
 * integrates with the LLD linker to extract component information during
 * the linking process. The plugin implements the PluginInterface and
 * provides functionality to:
 * - Detect and analyze object files and libraries
 * - Extract symbol and section information
 * - Generate SBOM data during linking
 * - Support various target architectures
 * 
 * Note: All plugin interface functions are implemented in LLDPlugin.cpp
 * to maintain separation of interface and implementation.
 */

#pragma once

// This header is intentionally left blank. All plugin interface functions are defined in LLDPlugin.cpp. 