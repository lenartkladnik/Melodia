#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <random>
#include <cctype>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <unordered_map>

#include "include/utils.hpp"
#include "include/storage_handler.hpp"
#include "include/data.hpp"

bool dark_mode = true;

const int ON_TOP = 999;
extern const size_t MAX_PAST_QUEUE_SIZE = 999;

const float padding_top = 100.f;
const float offset = 50.f;
const int in_round = 8;
const int out_round = 6;
const int main_n = 100;
const float progress_height = 8.f;
const int progress_round = 4;
const int progress_n = 4;
const int vol_round = 3;
const int vol_n = 4;
const float shadow_offset = 10.f;
const float small_shadow_offset = 5.f;
const float slider_threshold = 0.015;
const float queue_cover_size = 100.f;
const float selector_cover_size = 200.f;
const sf::Vector2f selector_size = {350.f, selector_cover_size};
const unsigned int queue_items = 6;
const float move_speed = 20.f;
const int match_diff = 2;
const int player_search_max_char = 28;
const int playlist_search_max_char = 42;
int input_max_char = 0;
const int queue_max_char = 26;
const float queue_contracted_width = 50.f;
const float control_corner_gap = 15.f;
const float scroll_speed = 25.f;

// Multiply the characters/font size values by a constant
// Tested fonts:
// Inter = 1
// Dongle = 1.8
const float font_multiplier = 1;

const float small_font_size = (18 * font_multiplier);
const float medium_font_size = (20 * font_multiplier);
const float medium_2_font_size = (22 * font_multiplier);
const float large_font_size = (24 * font_multiplier);

const sf::Vector2u window_base_size({1920, 1080});
const sf::ContextSettings window_settings{.antiAliasingLevel = 8};
sf::RenderWindow render_window;
sf::RenderTexture window;
sf::Vector2f window_size = {static_cast<float>(render_window.getSize().x), static_cast<float>(render_window.getSize().y)};
sf::View default_view = render_window.getDefaultView();
bool is_fullscreen;

int global_z_index = 0;

extern const std::unordered_map<std::string, sf::Vector2u> icon_sizes({
  {"volume", {30, 30}},
  {"trash", {26, 26}},
  {"side_expand", {48, 48}},
  {"side_contract", {48, 48}},
  {"previous", {26, 26}},
  {"plus", {26, 26}},
  {"play", {26, 26}},
  {"pause", {26, 26}},
  {"next", {26, 26}},
  {"mute", {30, 30}},
  {"manage_playlist", {32, 32}},
  {"live_full", {32, 32}},
  {"live_empty", {32, 32}},
  {"favorite_full", {32, 32}},
  {"favorite_empty", {32, 32}},
  {"edit", {26, 26}},
  {"download", {32, 32}},
  {"cancel", {26, 26}}
});

sf::Color main_color;
sf::Color dark_main_color;
sf::Color background_color;
sf::Color dark_background_color;
sf::Color light_background_color;
sf::Color lighter_background_color;
sf::Color background_shadow_color;
sf::Color dark_background_shadow_color;
sf::Color background_shadow_color_transparent;
sf::Color dark_background_shadow_color_transparent;
sf::Color progress_color;
sf::Color progress_done_color;
sf::Color text_color;
sf::Color cursor_color;
sf::Color light_text_color;
sf::Color lighter_text_color;
sf::Color white_color;
sf::Color title_color;
sf::Color artist_color;
sf::Color selection_color;
sf::Color cancel_area_color;
sf::Color volume_slider_color;

extern const int hover_sub = 20;
extern const uint8_t black_threshold = 20;
extern const uint8_t soft_white = 220;

extern const std::string inverted_image_suffix = "-inverted";

const sf::Cursor default_cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow).value();
const sf::Cursor text_cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Text).value();
const sf::Cursor hand_cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Hand).value();

sf::Font default_font;

bool held_left_mb_down = false;
std::vector<int> search_results = {};
int dragging_search_result = -1;
std::string progress_bar_string = "";
std::string progress_bar_doing_string = "";
float progress_bar_amount = 0.f;
float progress_bar_total = 0.f;
std::unique_ptr<std::thread> download_song_thread;
bool pause_main_input_handling = false;
float playlist_search_entry_height = queue_cover_size + 10.f;
float playlist_search_entry_unit = playlist_search_entry_height + 10.f;
float playlist_search_scroll_lower_bound = -playlist_search_entry_unit / 2;
float playlist_sel_scroll = playlist_search_scroll_lower_bound;
bool can_search_string_scroll = false;
bool search_was_active = false;

std::random_device rd;
std::mt19937 rand_generator(rd());

void set_colors() {
  white_color = adjust_if_dark_mode({212, 212, 212});
  main_color = adjust_if_dark_mode({232, 224, 209});
  dark_main_color = add_int_to_color(main_color, -5);

  background_color = adjust_if_dark_mode({227, 219, 211});
  dark_background_color = add_int_to_color(background_color, -5);
  light_background_color = adjust_if_dark_mode({217, 211, 200});
  lighter_background_color = adjust_if_dark_mode({0, 0, 0, 5});

  background_shadow_color = add_int_to_color(background_color, -10);
  dark_background_shadow_color = add_int_to_color(dark_background_color, -10);
  background_shadow_color_transparent = sf::Color({background_shadow_color.r, background_shadow_color.g, background_shadow_color.b, 128});
  dark_background_shadow_color_transparent = sf::Color({dark_background_shadow_color.r, dark_background_shadow_color.g, dark_background_shadow_color.b, 128});

  progress_color = adjust_if_dark_mode({180, 180, 180});
  progress_done_color = adjust_if_dark_mode({32, 32, 32});

  text_color = adjust_if_dark_mode({10, 10, 10});
  cursor_color = adjust_if_dark_mode({40, 40, 40});
  light_text_color = adjust_if_dark_mode({80, 80, 80});
  lighter_text_color = adjust_if_dark_mode({120, 120, 120});

  title_color = text_color;
  artist_color = light_text_color;

  if (dark_mode) {
    selection_color = {74, 121, 176};
  } else {
    selection_color = {181, 215, 255};
  }

  cancel_area_color = adjust_if_dark_mode({255, 0, 0, 20});

  volume_slider_color = adjust_if_dark_mode({10, 10, 10});
}
