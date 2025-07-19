#pragma once
#include <vector>
#include <algorithm>

namespace taskmgr {

template<typename T, typename Predicate>
std::vector<T> filter(const std::vector<T>& items, Predicate pred) {
    std::vector<T> result;
    std::copy_if(items.begin(), items.end(), std::back_inserter(result), pred);
    return result;
}

} // namespace taskmgr


