// LLVM symbols for plugin compatibility
#include <cstddef>

namespace llvm {
    __attribute__((visibility("default"))) bool DisableABIBreakingChecks = false;
} 