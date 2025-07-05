#include "GoldAdapter.hpp"
#include <iostream>
#include <memory>

namespace {
    static std::unique_ptr<heimdall::GoldAdapter> globalAdapter;
}

extern "C" {
int onload(void* tv)
{
    std::cout << "Heimdall Gold Plugin activated" << std::endl;
    globalAdapter = std::make_unique<heimdall::GoldAdapter>();
    globalAdapter->initialize();
    return 0;
}

const char* heimdall_gold_version()
{
    return "1.0.0";
}

const char* heimdall_gold_description()
{
    return "Heimdall SBOM Generator Plugin for GNU Gold Linker";
}
}
