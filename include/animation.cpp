#include <SFML/Graphics.hpp>
#include <thread>
#include <time.h>
#include <vector>
#include <algorithm>
#include "data.hpp"

struct AnimateThread {
  sf::Transformable* transformable_ptr;
  std::thread* thread;
  bool running;
};

std::vector<AnimateThread> animate_move_x_running;

void animate_move_x(sf::Transformable& transformable, float target, float step, bool* flag = nullptr, bool set_flag_to = true, AnimationStage flag_stage = AnimationStage::end) {
  if (animate_move_x_running.size() >= 2) return; // Too many operations are already happening

  float speed = 0.01;
  auto move = [&transformable, target, step, speed, set_flag_to, flag_stage](size_t index, bool* flag) {
    while (animate_move_x_running.size() <= index) {} // wait for the AnimateThread to be appended

    int found = 0;
    int found_counter = 0;
    for (auto each : animate_move_x_running) {
      if (&transformable == each.transformable_ptr) {
        found_counter++;
      }
      if (found_counter == 0) found++;
    }

    if (found_counter > 1) {
      size_t len_before = animate_move_x_running.size();
      animate_move_x_running[found].running = false;
      while (len_before == animate_move_x_running.size()) {} // wait for the erase at the end of move
    }

    float total = std::max(transformable.getPosition().x - target, target - transformable.getPosition().x);
    if (std::abs(step) > total && flag != nullptr) *flag = set_flag_to;
    auto step_clamp = std::clamp(step, -total, total);

    if (flag_stage == AnimationStage::start && flag != nullptr) *flag = set_flag_to;

    std::clock_t start;
    for (float i = 0; i < total; i += std::abs(step_clamp)) {
      // recalculate the index before use, since it can change at any time
      index = 0;
      for (auto each : animate_move_x_running) {
        if (&transformable == each.transformable_ptr) {
          break;
        }
        index++;
      }
      if (!animate_move_x_running[index].running) break;

      start = std::clock();

      transformable.move({step_clamp, 0.f});

      auto diff = ((float)(std::clock() - start))/CLOCKS_PER_SEC;
      while (diff < speed) diff = ((float)(std::clock() - start))/CLOCKS_PER_SEC;

      if (flag_stage == AnimationStage::half && ((i >= total / 2) && (i - step_clamp < total / 2)) && flag != nullptr) *flag = set_flag_to;
    }

    // recalculate the index before use, since it can change at any time
    index = 0;
    for (auto each : animate_move_x_running) {
      if (&transformable == each.transformable_ptr) {
        break;
      }
      index++;
    }
    animate_move_x_running.erase(animate_move_x_running.begin() + index);

    if (flag_stage == AnimationStage::end && flag != nullptr) *flag = set_flag_to;
  };

  size_t index = animate_move_x_running.size();
  std::thread t(move, index, flag);
  animate_move_x_running.push_back(AnimateThread{&transformable, &t, true});
  t.detach();
}

void animate_move_all_x(std::vector<sf::Transformable*> transformables, float target, float step, bool* flag = nullptr, bool set_flag_to = true, AnimationStage flag_stage = AnimationStage::end) {
  for (auto transformable : transformables) {
    animate_move_x(*transformable, target, step, flag, set_flag_to, flag_stage);
  }
}
