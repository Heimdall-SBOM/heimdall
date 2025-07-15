#pragma once
#include <string>
#include <vector>
#include "Task.h"

namespace taskmgr {

class Project {
public:
    Project(int id, const std::string& name);
    void addTask(const Task& task);
    const std::vector<Task>& getTasks() const;
    int getId() const;
    const std::string& getName() const;
    void print() const;
private:
    int id;
    std::string name;
    std::vector<Task> tasks;
};

} // namespace taskmgr

