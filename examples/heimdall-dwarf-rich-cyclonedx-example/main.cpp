#include "TaskManager.h"
#include "User.h"
#include "Project.h"
#include "Task.h"
#include "utils.h"
#include <iostream>
#include <functional>

using namespace taskmgr;

int main() {
    TaskManager mgr;
    // Create users
    mgr.addUser(User(1, "Alice"));
    mgr.addUser(User(2, "Bob"));
    mgr.addUser(User(3, "Charlie"));

    // Create projects
    Project proj1(101, "Heimdall SBOM");
    Project proj2(102, "DWARF Demo");
    mgr.addProject(proj1);
    mgr.addProject(proj2);

    // Create tasks
    Task t1(1001, "Implement parser", "Write the parser for SBOM extraction");
    Task t2(1002, "Write tests", "Add unit tests for DWARF extraction");
    Task t3(1003, "Document API", "Write API documentation");
    Task t4(1004, "Refactor code", "Improve code structure");
    t2.setStatus(TaskStatus::InProgress);
    t3.setStatus(TaskStatus::Blocked);
    t4.setStatus(TaskStatus::Done);

    // Assign tasks to projects
    mgr.assignTaskToProject(101, t1);
    mgr.assignTaskToProject(101, t2);
    mgr.assignTaskToProject(102, t3);
    mgr.assignTaskToProject(102, t4);

    // Print summary
    mgr.printSummary();

    // Use template filter utility
    std::vector<Task> allTasks = mgr.findTasks([](const Task&){ return true; });
    auto doneTasks = filter(allTasks, [](const Task& t){ return t.getStatus() == TaskStatus::Done; });
    std::cout << "\n[Done Tasks]" << std::endl;
    for (const auto& t : doneTasks) t.print();

    // Use std::function and findTasks
    std::function<bool(const Task&)> isBlocked = [](const Task& t){ return t.getStatus() == TaskStatus::Blocked; };
    auto blockedTasks = mgr.findTasks(isBlocked);
    std::cout << "\n[Blocked Tasks]" << std::endl;
    for (const auto& t : blockedTasks) t.print();

    return 0;
}

