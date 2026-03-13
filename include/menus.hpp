#ifndef MENUS_HPP
#define MENUS_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include "RoundedRectangleShape.hpp"
#include "data.hpp"

void switch_to_player(MenuData& menu_data);
StaticPlayerData init_player(sf::RenderWindow& window, const std::string& song_path, const std::string& playlist);
void display_player(MenuData::PlayerData& menu, sf::RenderWindow& window);

#endif
