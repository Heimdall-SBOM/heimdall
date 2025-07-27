#include "User.h"
#include <iostream>
#include <utility>

namespace taskmgr
{

User::User(int id, std::string name) : id(id), name(std::move(name)) {}
int User::getId() const
{
   return id;
}
const std::string& User::getName() const
{
   return name;
}
void User::print() const
{
   std::cout << "[User] #" << id << ": " << name << "\n";
}

}  // namespace taskmgr
