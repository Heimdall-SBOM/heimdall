#include "TaskManager.h"
#include <algorithm>
#include <iostream>

namespace taskmgr
{

TaskManager::TaskManager() = default;

void TaskManager::addUser(const User& user)
{
   users.push_back(user);
}

void TaskManager::addProject(const Project& project)
{
   projects.push_back(project);
}

void TaskManager::assignTaskToProject(int projectId, const Task& task)
{
   auto it = std::find_if(projects.begin(), projects.end(),
                          [projectId](const Project& p) { return p.getId() == projectId; });
   if (it != projects.end())
   {
      it->addTask(task);
   }
}

void TaskManager::printSummary() const
{
   std::cout << "[TaskManager] Users: " << users.size() << ", Projects: " << projects.size()
             << "\n";
   for (const auto& user : users)
      user.print();
   for (const auto& project : projects)
      project.print();
}

}  // namespace taskmgr
