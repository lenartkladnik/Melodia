#ifndef PLAYLIST_SELECTOR_MENU_HPP
#define PLAYLIST_SELECTOR_MENU_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include "../../external/lib/RoundedRectangleShape.hpp"
#include "data.hpp"

std::shared_ptr<StaticPlaylistSelectorData> init_playlist_selector(sf::RenderTexture& window, sf::RenderWindow& render_window, MenuData& menu_data);
bool display_playlist_selector(MenuData::PlaylistSelectorData& playlist_sel, sf::RenderTexture& window, sf::RenderWindow& render_window, MenuData& menu_data);
void switch_to_playlist_selector(MenuData& menu_data, sf::RenderTexture& window, sf::RenderWindow& render_window);

#endif
