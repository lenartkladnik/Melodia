#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <vector>
#include <functional>

class Task;

extern std::vector<Task> tasks;

class Task {
  private:
    std::function<void()> function;
    size_t frame_timer;
    bool done = false;

  public:
    Task(
      std::function<void()> function,
      size_t n_frames
    )
      : function(function),
        frame_timer(n_frames)
    {};

    bool is_done() const {
      return done;
    }

    void tick() {
      if (frame_timer == 0) {
        function();
        done = true;

        return;
      }

      frame_timer--;
    }
};

void remove_done_tasks();

void wait_n_frames(std::function<void()> function);
void on_next_frame(std::function<void()> function);

#endif
