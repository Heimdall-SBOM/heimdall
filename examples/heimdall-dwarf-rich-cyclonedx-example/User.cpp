#include "User.h"
#include <iostream>

namespace taskmgr {

User::User(int id, const std::string& name) : id(id), name(name) {}
int User::getId() const { return id; }
const std::string& User::getName() const { return name; }
void User::print() const {
    std::cout << "[User] #" << id << ": " << name << "\n";
}

} // namespace taskmgr

