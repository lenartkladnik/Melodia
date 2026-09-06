#ifndef PLAYLIST_SELECTOR_MENU_HPP
#define PLAYLIST_SELECTOR_MENU_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include "../../external/lib/RoundedRectangleShape.hpp"
#include "data.hpp"

std::shared_ptr<StaticPlaylistSelectorData> init_playlist_selector();
bool display_playlist_selector();
void switch_to_playlist_selector();

#endif
