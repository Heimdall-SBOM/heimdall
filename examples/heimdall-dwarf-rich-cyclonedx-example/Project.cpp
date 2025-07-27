#include "Project.h"
#include <iostream>
#include <utility>

namespace taskmgr
{

Project::Project(int id, std::string name) : id(id), name(std::move(name)) {}

void Project::addTask(const Task& task)
{
   tasks.push_back(task);
}

const std::vector<Task>& Project::getTasks() const
{
   return tasks;
}

int Project::getId() const
{
   return id;
}
const std::string& Project::getName() const
{
   return name;
}

void Project::print() const
{
   std::cout << "[Project] #" << id << ": " << name << "\n";
   for (const auto& task : tasks)
   {
      task.print();
   }
}

}  // namespace taskmgr
