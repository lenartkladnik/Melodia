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
#include "include/storage_handler.hpp"
#include "include/data.hpp"

extern const int ON_TOP = 999;

extern const float padding_top = 100.f;
extern const float offset = 50.f;
extern const int in_round = 8;
extern const int out_round = 6;
extern const int main_n = 100;
extern const float progress_height = 8.f;
extern const int progress_round = 4;
extern const int progress_n = 4;
extern const int vol_round = 3;
extern const int vol_n = 4;
extern const float shadow_offset = 10.f;
extern const float small_shadow_offset = 5.f;
extern const float slider_threshold = 0.015;
extern const float queue_cover_size = 100.f;
extern const float selector_cover_size = 200.f;
extern const sf::Vector2f selector_size = {350.f, selector_cover_size};
extern const int queue_items = 6;
extern const float move_speed = 20.f;
extern const int match_diff = 2;
extern const int player_search_max_char = 28;
extern const int playlist_search_max_char = 42;
int input_max_char = 0;
extern const int queue_max_char = 26;
extern const float queue_contracted_width = 50.f;
extern const float control_corner_gap = 15.f;
extern const float scroll_speed = 25.f;

extern const float font_multiplier = 1; // Multiply the font values (this exists purely for easier changing between fonts whilst developing)
                                          // Inter: 1
                                          // Dongle: 1.8
extern const float small_font_size = (18 * font_multiplier);
extern const float medium_font_size = (20 * font_multiplier);
extern const float medium_2_font_size = (22 * font_multiplier);
extern const float large_font_size = (24 * font_multiplier);

extern const sf::Vector2u window_base_size({1920, 1080});
extern const sf::ContextSettings window_settings{.antiAliasingLevel = 8};
sf::RenderWindow window;
sf::Vector2f window_size = {static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)};
sf::View default_view = window.getDefaultView();
bool is_fullscreen;

int global_z_index = 0;

extern const sf::Color main_color({232, 224, 209});
extern const sf::Color dark_main_color({main_color.r - 5, main_color.g - 5, main_color.b - 5}); // ({209, 204, 194});
extern const sf::Color background_color({227, 219, 211}); //({196, 186, 189});
extern const sf::Color dark_background_color({background_color.r - 5, background_color.g - 5, background_color.b - 5}); // ({156, 146, 149});
extern const sf::Color light_background_color({217, 211, 200});
extern const sf::Color lighter_background_color({0, 0, 0, 5});
extern const sf::Color background_shadow_color({background_color.r - 20, background_color.g - 20, background_color.b - 20}); // ({176, 166, 169});
extern const sf::Color dark_background_shadow_color({dark_background_color.r - 10, dark_background_color.g - 10, dark_background_color.b - 10}); // ({153, 144, 147});
extern const sf::Color background_shadow_color_transparent({background_shadow_color.r, background_shadow_color.g, background_shadow_color.b, 128});
extern const sf::Color dark_background_shadow_color_transparent({dark_background_shadow_color.r, dark_background_shadow_color.g, dark_background_shadow_color.b, 128});
extern const sf::Color progress_color({180, 180, 180});
extern const sf::Color progress_done_color({32, 32, 32});
extern const sf::Color text_color({10, 10, 10});
extern const sf::Color cursor_color({40, 40, 40});
extern const sf::Color light_text_color({80, 80, 80});
extern const sf::Color lighter_text_color({120, 120, 120});
extern const sf::Color white_color({212, 212, 212});
extern const sf::Color title_color = text_color;
extern const sf::Color artist_color = light_text_color;
extern const sf::Color selection_color({181, 215, 255});

extern const sf::Color hover_sub({20, 20, 20}); // TODO: Replace this with int since sf::Color - sf::Color doesn't work

extern const sf::Cursor default_cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow).value();
extern const sf::Cursor text_cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Text).value();
extern const sf::Cursor hand_cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Hand).value();

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
