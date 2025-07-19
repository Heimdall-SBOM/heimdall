#include "Task.h"
#include <iostream>

namespace taskmgr {

Task::Task(int id, const std::string& title, const std::string& desc)
    : id(id), title(title), description(desc), status(TaskStatus::Todo) {}

Task::~Task() {}

int Task::getId() const { return id; }
const std::string& Task::getTitle() const { return title; }
const std::string& Task::getDescription() const { return description; }
TaskStatus Task::getStatus() const { return status; }
void Task::setStatus(TaskStatus s) { status = s; }
void Task::setDueDate(const std::chrono::system_clock::time_point& due) { dueDate = due; }
std::chrono::system_clock::time_point Task::getDueDate() const { return dueDate; }

void Task::print() const {
    std::cout << "[Task] #" << id << ": " << title << " (" << description << ")\n";
}

} // namespace taskmgr

