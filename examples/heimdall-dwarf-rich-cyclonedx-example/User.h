#pragma once
#include <string>

namespace taskmgr
{

class User
{
  public:
      User(int id, std::string name);
      int                getId() const;
      const std::string& getName() const;
      void               print() const;

  private:
  int         id;
  std::string name;
};

}  // namespace taskmgr
