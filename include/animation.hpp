#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include <SFML/Graphics.hpp>
#include "data.hpp"

void animate_move_x(sf::Transformable& transformable, float total, float step, bool* flag = nullptr, bool set_flag_to = true, AnimationStage flag_stage = AnimationStage::end);
void animate_move_all_x(std::vector<sf::Transformable*> transformables, float target, float step, bool* flag = nullptr, bool set_flag_to = true, AnimationStage flag_stage = AnimationStage::end);

#endif
