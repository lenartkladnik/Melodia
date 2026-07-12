#ifndef UTILS_HPP
#define UTILS_HPP

#include <SFML/Graphics.hpp>
#include <string>

void debug_draw_bounds(sf::RenderWindow& window, sf::FloatRect bounds);

std::shared_ptr<sf::Texture> load_texture(std::string path);

#endif
