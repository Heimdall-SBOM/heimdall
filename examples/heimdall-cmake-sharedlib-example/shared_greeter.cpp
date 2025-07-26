#include "shared_greeter.h"

std::string shared_greet(const std::string& name)
{
  return "[Shared] Hello, " + name + "!";
}