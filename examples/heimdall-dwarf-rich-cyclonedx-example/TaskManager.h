#pragma once
#include <vector>
#include <memory>
#include "User.h"
#include "Project.h"

namespace taskmgr {

class TaskManagerBase {
public:
    virtual ~TaskManagerBase() = default;
    virtual void printSummary() const = 0;
};

class TaskManager : public TaskManagerBase {
public:
    TaskManager();
    void addUser(const User& user);
    void addProject(const Project& project);
    void assignTaskToProject(int projectId, const Task& task);
    void printSummary() const override;
    template<typename Predicate>
    std::vector<Task> findTasks(Predicate pred) const {
        std::vector<Task> result;
        for (const auto& project : projects) {
            for (const auto& task : project.getTasks()) {
                if (pred(task)) result.push_back(task);
            }
        }
        return result;
    }
private:
    std::vector<User> users;
    std::vector<Project> projects;
};

} // namespace taskmgr
