#ifndef UTILS_HPP
#define UTILS_HPP

#include <SFML/Graphics.hpp>
#include <string>

void debug_draw_bounds(sf::RenderWindow& window, sf::FloatRect bounds);

std::shared_ptr<sf::Texture> load_texture(std::string path);
bool is_string_valid_name(const std::string& s);
void mkdir(std::string path);
bool must_exist(std::string path);
std::string get_stem(std::string path);
std::string insert_in_stem(std::string path, std::string s);
std::string get_next_available_path(std::string path);

#endif
