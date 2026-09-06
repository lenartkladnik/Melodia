#ifndef PLAYER_MENU_HPP
#define PLAYER_MENU_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include "../../external/lib/RoundedRectangleShape.hpp"
#include "data.hpp"

void switch_to_player(std::string playlist);
std::shared_ptr<StaticPlayerData> init_player(const std::string& song_path, int id, const std::string& playlist);
void display_player();

void done_playing(std::vector<int>& playlist, std::vector<int>& past_queue);
int get_start_song(std::vector<int>& playlist);

#endif
