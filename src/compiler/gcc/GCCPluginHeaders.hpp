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
 * @file GCCPluginHeaders.hpp
 * @brief Isolated GCC plugin headers to prevent conflicts
 * @author Trevor Bakker
 * @date 2025
 *
 * This file provides isolated access to GCC plugin headers while
 * preventing conflicts with other libraries like nlohmann JSON.
 */

#pragma once

// Save any existing problematic macros
#ifdef abort
#define HEIMDALL_SAVED_ABORT abort
#undef abort
#endif

#ifdef assert
#define HEIMDALL_SAVED_ASSERT assert
#undef assert
#endif

// Include GCC plugin headers in an isolated context
extern "C" {
    #include "gcc-plugin.h"
    #include "plugin-version.h"
    #include "tree.h"
    #include "c-family/c-common.h"
    #include "diagnostic.h"
    #include "plugin.h"
}

// Restore standard library functions
#include <cstdlib>
#include <cassert>

// Restore the original macros if they were saved
#ifdef HEIMDALL_SAVED_ABORT
#define abort HEIMDALL_SAVED_ABORT
#undef HEIMDALL_SAVED_ABORT
#endif

#ifdef HEIMDALL_SAVED_ASSERT
#define assert HEIMDALL_SAVED_ASSERT
#undef HEIMDALL_SAVED_ASSERT
#endif 