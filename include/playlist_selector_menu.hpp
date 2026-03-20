#ifndef PLAYLIST_SELECTOR_MENU_HPP
#define PLAYLIST_SELECTOR_MENU_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include "RoundedRectangleShape.hpp"
#include "data.hpp"

std::unique_ptr<StaticPlaylistSelectorData> init_playlist_selector(sf::RenderWindow& window);
bool display_playlist_selector(MenuData::PlaylistSelectorData& playlist_sel, sf::RenderWindow& window, MenuData& menu_data);
void switch_to_playlist_selector(MenuData& menu_data, sf::RenderWindow& window);

#endif
