#include "Task.h"
#include <iostream>
#include <utility>

namespace taskmgr
{

Task::Task(int id, std::string title, std::string desc)
   : id(id), title(std::move(title)), description(std::move(desc)), status(TaskStatus::Todo)
{
}

Task::~Task() = default;

int Task::getId() const
{
   return id;
}
const std::string& Task::getTitle() const
{
   return title;
}
const std::string& Task::getDescription() const
{
   return description;
}
TaskStatus Task::getStatus() const
{
   return status;
}
void Task::setStatus(TaskStatus s)
{
   status = s;
}
void Task::setDueDate(const std::chrono::system_clock::time_point& due)
{
   dueDate = due;
}
std::chrono::system_clock::time_point Task::getDueDate() const
{
   return dueDate;
}

void Task::print() const
{
   std::cout << "[Task] #" << id << ": " << title << " (" << description << ")\n";
}

}  // namespace taskmgr
