#ifndef UTILS_HPP
#define UTILS_HPP

#include <SFML/Graphics.hpp>
#include <string>

void debug_draw_bounds(sf::RenderWindow& window, sf::FloatRect bounds);

void set_window(sf::State state);
void new_random();
std::string exec(const char* cmd);
std::shared_ptr<sf::Texture> load_texture(std::string path);
bool is_string_valid_name(const std::string& s);
void mkdir(std::string path);
bool must_exist(std::string path);
std::string get_stem(std::string path);
std::string insert_in_stem(std::string path, std::string s);
std::string get_next_available_path(std::string path);
sf::Vector2f find_character_pos(const sf::Text& text, size_t index);
sf::Vector2f find_character_size(const sf::Text& text, size_t index);
size_t find_character_at_pos_x(const std::u32string& string, const sf::Text& text, float pos_x);
inline sf::Vector2f get_mouse_pos(sf::RenderWindow& window) { return window.mapPixelToCoords(sf::Mouse::getPosition(window)); }
std::string u32_to_utf8(const std::u32string& u32);
std::u32string utf8_to_u32(const std::string& utf8);
bool matching(std::u32string s1, std::u32string s2, int threshold);
std::string seconds_to_human_readable(float seconds);
float getFontOffsetPixels(float target_size);
void setFontSize(sf::Text& text, float target_size, unsigned int raster_mul = 2);
void reset_globals();

#endif
