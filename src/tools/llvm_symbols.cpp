// LLVM symbols for plugin compatibility
#include <cstddef>

namespace llvm {
    __attribute__((visibility("default"))) const bool DisableABIBreakingChecks = false;
} 