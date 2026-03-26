#ifndef PLAYER_MENU_HPP
#define PLAYER_MENU_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include "RoundedRectangleShape.hpp"
#include "data.hpp"

void switch_to_player(MenuData& menu_data, std::string playlist, sf::Font& default_font);
std::unique_ptr<StaticPlayerData> init_player(sf::RenderWindow& window, const std::string& song_path, const std::string& playlist, sf::Font& default_font);
void display_player(MenuData::PlayerData& menu, sf::RenderWindow& window, sf::Font& default_font);

#endif
