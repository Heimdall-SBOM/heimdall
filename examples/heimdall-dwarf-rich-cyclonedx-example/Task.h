#pragma once
#include <string>
#include <chrono>

namespace taskmgr {

enum class TaskStatus { Todo, InProgress, Done, Blocked };

class Task {
public:
    Task(int id, const std::string& title, const std::string& desc);
    virtual ~Task();

    int getId() const;
    const std::string& getTitle() const;
    const std::string& getDescription() const;
    TaskStatus getStatus() const;
    void setStatus(TaskStatus status);
    void setDueDate(const std::chrono::system_clock::time_point& due);
    std::chrono::system_clock::time_point getDueDate() const;

    virtual void print() const;

protected:
    int id;
    std::string title;
    std::string description;
    TaskStatus status;
    std::chrono::system_clock::time_point dueDate;
};

} // namespace taskmgr

