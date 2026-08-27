#include <functional>
#include <algorithm>

#include "include/scheduler.hpp"

std::vector<Task> tasks;

void remove_done_tasks() {
  tasks.erase(std::remove_if(tasks.begin(), tasks.end(),
    [](const Task& t){ return t.is_done(); }),
  tasks.end());
}

void wait_n_frames(size_t n, std::function<void(MenuData&)> function) {
  tasks.emplace_back(function, n);
}

void on_next_frame(std::function<void(MenuData&)> function) {
  tasks.emplace_back(function, 1);
}
