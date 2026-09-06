#ifndef PLAYLIST_SELECTOR_MENU_HPP
#define PLAYLIST_SELECTOR_MENU_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include "../../external/lib/RoundedRectangleShape.hpp"
#include "data.hpp"

std::shared_ptr<StaticPlaylistSelectorData> init_playlist_selector(MenuData&);
bool display_playlist_selector(MenuData::PlaylistSelectorData& playlist_sel, MenuData&);
void switch_to_playlist_selector(MenuData&);

#endif
