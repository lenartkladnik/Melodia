#ifndef MENUS_HPP
#define MENUS_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include "RoundedRectangleShape.hpp"
#include "data.hpp"

std::unique_ptr<StaticData> init_general(sf::RenderWindow& window, sf::Vector2f search_size, sf::Vector2f search_pos, sf::Font& default_font);

void search_reset_cursor();
void search_move_cursor_left();
void search_move_cursor_right();
void search_running(sf::Text& search_before_cursor, sf::Text& search_after_cursor);
void search_input(char32_t input);
void search_focus(sf::Vector2f& pos, sf::Text& search_before_cursor, sf::Text& search_after_cursor);
void search_unfocus(sf::Text& search_before_cursor, sf::Text& search_after_cursor);
void search_draw_cursor(sf::RenderWindow& window, sf::Text& search_before_cursor, sf::RoundedRectangleShape& search_background);

#endif
