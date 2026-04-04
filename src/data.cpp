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
#include <sys/stat.h>
#include "../external/lib/nlohmann/json.hpp"
#include "include/data.hpp"
#include "../external/lib/httplib.h"

#ifndef _WIN32
  #define STB_IMAGE_IMPLEMENTATION
  #define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include "stb/stb_image.h"
#include "stb/stb_image_resize2.h"
#include "stb/stb_image_write.h"

#ifdef _WIN32
  #define POPEN _popen
  #define PCLOSE _pclose
#else
  #define POPEN popen
  #define PCLOSE pclose
#endif

extern const float padding_top = 100.f;
extern const float offset = 50.f;
extern const int in_round = 8;
extern const int out_round = 6;
extern const int main_n = 40;
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
extern const std::string base_path = "./";
extern const std::string base_path_misc = base_path + "misc/";
extern const std::string base_path_external = base_path + "external/";
extern const std::string base_path_external_prog = base_path_external + "prog/";
extern const std::string base_music_path = ".music_data/";
extern const std::string base_music_path_data = base_music_path + "data/";
extern const std::string base_music_path_playlists = base_music_path + "playlists/";
extern const int queue_items = 6;
extern const float move_speed = 20.f;
extern const float match_diff = 3.f;
extern const int player_search_max_char = 28;
extern const int playlist_search_max_char = 42;
int search_max_char = 0;
extern const int queue_max_char = 26;
extern const float queue_contracted_width = 50.f;
extern const float control_corner_gap = 15.f;

extern const sf::Vector2u window_base_size({1920, 1080});
sf::RenderWindow window(sf::VideoMode(window_base_size), "Melodia", sf::Style::Default, sf::State::Windowed);
extern const sf::Vector2u window_size = window.getSize();

extern const sf::Color main_color({179, 126, 25});
extern const sf::Color dark_main_color({156, 110, 22});
extern const sf::Color background_color({196, 186, 189});
extern const sf::Color dark_background_color({156, 146, 149});
extern const sf::Color background_shadow_color({176, 166, 169});
extern const sf::Color dark_background_shadow_color({153, 144, 147});
extern const sf::Color background_shadow_color_transparent({background_shadow_color.r, background_shadow_color.g, background_shadow_color.b, 128});
extern const sf::Color dark_background_shadow_color_transparent({dark_background_shadow_color.r, dark_background_shadow_color.g, dark_background_shadow_color.b, 128});
extern const sf::Color progress_color(212, 201, 204);
extern const sf::Color text_color({10, 10, 10});
extern const sf::Color light_text_color({80, 80, 80});
extern const sf::Color white_color({212, 212, 212});
extern const sf::Color title_color = text_color;
extern const sf::Color artist_color = light_text_color;

extern const sf::Cursor default_cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow).value();
extern const sf::Cursor text_cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Text).value();
extern const sf::Cursor hand_cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Hand).value();

bool search_active = false;
bool show_cursor = false;
size_t cursor_pos = 0;
bool held_left_mb_down = false;
bool reset_cursor = true;
std::string search_string = "";
std::string prev_search_string = "";
std::vector<int> search_results = {};
std::string progress_bar_string = "";
float progress_bar_amount = 0.f;
float progress_bar_total = 0.f;
std::unique_ptr<std::thread> download_song_thread;
bool pause_main_input_handling = false;

std::vector<ClickEvent> click_events;
std::vector<ClickEvent> search_res_click_events;

std::random_device rd;
std::mt19937 rand_generator(rd());

void new_random() {
  rand_generator.seed(rd());
}

// Source - https://stackoverflow.com/a/478960
// Posted by waqas, modified by community. See post 'Timeline' for change history
// Retrieved 2026-03-28, License - CC BY-SA 4.0
std::string exec(const char* cmd) {
  std::cout << "Info: Executing '" << cmd << "'." << std::endl;

  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&PCLOSE)> pipe(POPEN(cmd, "r"), PCLOSE);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

// ==============================================================================

void new_click_event(std::vector<ClickEvent>& container, std::function<void(MenuData&)> function, sf::FloatRect bounds, sf::Mouse::Button mouse_button) {
  for (const auto& each : container) {
    if (each.bounds == bounds) return;
  }

  container.push_back(ClickEvent{function, bounds, mouse_button});
}

std::string construct_song_path(int id) {
  return base_music_path_data + std::to_string(id);
}

std::string get_song_title(int id) {
  auto song_path = construct_song_path(id);

  std::ifstream title_file(song_path + ".title");
  std::string title_string = "";
  if (title_file.good()) {
    std::getline(title_file, title_string);
  } else {
    std::cerr << "Error: Failed to read title from '" << song_path << ".title" << "'.";
  }

  return title_string;
}

std::string get_song_artist(int id) {
  auto song_path = construct_song_path(id);

  std::ifstream artist_file(song_path + ".artist");
  std::string artist_string = "";
  if (artist_file.good()) {
    std::getline(artist_file, artist_string);
  } else {
    std::cerr << "Error: Failed to read artist name from '" << song_path << ".artist" << "'.";
  }

  return artist_string;
}

std::vector<std::string> get_all_playlists() {
  std::vector<std::string> playlists;

  struct stat s;
  for (const auto& entry : std::filesystem::directory_iterator(base_music_path_playlists)) {
    auto path = entry.path();
    auto str_path = path.string();

    // First check that the path doesn't contain a dot, because the playlist files don't
    // have an extension. Then check that the path is a file and not a directory.
    if (path.filename().string().find(".") == std::string::npos && stat(str_path.c_str(), &s) == 0 && !(s.st_mode & S_IFDIR)) {
      playlists.push_back(path.stem().u8string());
    }
  }

  return playlists;
}

std::vector<int> get_playlist(const std::string& name) {
  std::vector<int> ids;
  std::string entry;
  std::ifstream playlist_file(base_music_path_playlists + name);

  while (std::getline(playlist_file, entry)) {
    int id = std::stoi(entry);

    if (std::find(ids.begin(), ids.end(), id) == ids.end())
      ids.push_back(id);
  }

  std::shuffle(ids.begin(), ids.end(), rand_generator);

  return ids;
}

void done_playing(std::vector<int>& playlist, std::vector<int>& past) {
  // pop the first element into id
  int id = playlist[0];

  past.push_back(id);
  if (past.size() > queue_items) past.erase(past.begin());

  playlist.erase(playlist.begin());

  if (playlist.size() <= queue_items) {
    playlist.push_back(id);
  }
  else {
    std::uniform_int_distribution<> distr(1, playlist.size() - queue_items - 1);
    playlist.insert(playlist.begin() + queue_items + distr(rand_generator), id); // Add queue_items to prevent the user from seeing the insertion
  }
}

int get_start_song(std::vector<int>& playlist) {
  int forbidden = playlist[0]; // the starting song can't be the first song in the queue

  if (playlist.size() == 1) {
    return playlist[0]; // ignore the forbidden id
  }

  std::uniform_int_distribution<> distr(0, playlist.size() - 1);

  int id = distr(rand_generator);
  int i = 0;
  while (id == forbidden) {
    id = distr(rand_generator);
    i++;
    if (i > 10) break;
  }

  if (i > 10) {
    // Should (almost) never happen. Produce a non random first song
    if (forbidden == 0) {
      id = 1;
    }
    else {
      id = forbidden - 1;
    }
  }

  return id;
}

std::string char32_to_utf8(char32_t c32) {
  std::string result;

  if (c32 <= 0x7F) {
    result += static_cast<char>(c32);
  } else if (c32 <= 0x7FF) {
    result += static_cast<char>(0xC0 | (c32 >> 6));
    result += static_cast<char>(0x80 | (c32 & 0x3F));
  } else if (c32 <= 0xFFFF) {
    result += static_cast<char>(0xE0 | (c32 >> 12));
    result += static_cast<char>(0x80 | ((c32 >> 6) & 0x3F));
    result += static_cast<char>(0x80 | (c32 & 0x3F));
  } else if (c32 <= 0x10FFFF) {
    result += static_cast<char>(0xF0 | (c32 >> 18));
    result += static_cast<char>(0x80 | ((c32 >> 12) & 0x3F));
    result += static_cast<char>(0x80 | ((c32 >> 6) & 0x3F));
    result += static_cast<char>(0x80 | (c32 & 0x3F));
  }

  return result;
}

std::string stripNonAlphaNum(const std::string& str) {
  std::string result;
  for (char ch : str) {
    if (std::isalnum(ch) || ch == ' ') {
      result += ch;
    }
  }
  return result;
}

bool isSubstring(const std::string& s1, const std::string& s2) {
  return s1.find(s2) != std::string::npos || s2.find(s1) != std::string::npos;
}

bool matching(const std::string& s1, const std::string& s2, float diff, const char split) {
    auto s1_clean = stripNonAlphaNum(s1);
    auto s2_clean = stripNonAlphaNum(s2);

    std::transform(s1_clean.begin(), s1_clean.end(), s1_clean.begin(), ::tolower);
    std::transform(s2_clean.begin(), s2_clean.end(), s2_clean.begin(), ::tolower);

    if (isSubstring(s1_clean, s2_clean)) {
      return true;
    }

    std::vector<std::string> s1_split, s2_split;
    std::stringstream ss1(s1_clean), ss2(s2_clean);
    std::string token;

    while (std::getline(ss1, token, split)) {
      s1_split.push_back(token);
    }

    while (std::getline(ss2, token, split)) {
      s2_split.push_back(token);
    }

    float score = 0;
    int matches = 0;

    for (size_t c = 0; c < s1_split.size(); c++) {
      for (size_t d = 0; d < s2_split.size(); d++) {
        const auto& i = s1_split[c];
        const auto& j = s2_split[d];

        // Calculate character difference
        std::string diff_i_j, diff_j_i;
        std::set_difference(i.begin(), i.end(), j.begin(), j.end(), std::inserter(diff_i_j, std::begin(diff_i_j)));
        std::set_difference(j.begin(), j.end(), i.begin(), i.end(), std::inserter(diff_j_i, std::begin(diff_j_i)));

        if (!(diff_i_j.size() + diff_j_i.size() <= diff)) {
          score += std::min(diff_i_j.size(), diff_j_i.size());
        }
        else {
          matches++;
        }
      }
    }
    score = score / matches; // inf if 0 otherwise the more matches there where the smaller the score

    if (score < diff) {
      return true;
    }

    return false;
}

std::string seconds_to_human_readable(float total_sec_left) {
  int minutes = (int)(total_sec_left / 60); // floor positive value
  std::string seconds = std::to_string((int)((int)total_sec_left % 60));

  if (seconds.size() < 2) {
    seconds = "0" + seconds;
  }

  return std::to_string(minutes) + ":" + seconds;
}

std::vector<int> search_all_songs(const std::string& query) {
  std::vector<int> results;

  struct stat s;
  for (const auto& entry : std::filesystem::directory_iterator(base_music_path_data)) {
    auto path = entry.path();
    auto str_path = path.string();

    // First check that the path contains the .mp3 extensions, since it is one of the
    // extensions every song has. Then check that the path is a file and not a directory.
    if (path.filename().string().find(".mp3") != std::string::npos && stat(str_path.c_str(), &s) == 0 && !(s.st_mode & S_IFDIR)) {
      int id = -1;
      try {
        id = std::stoi(path.stem().u8string());
      } catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
      } catch (const std::out_of_range& e) {
        std::cerr << e.what() << std::endl;
      }

      if (id >= 0) {
        if (matching(query, get_song_title(id), match_diff)) {
          results.push_back(id);
          continue; // Don't bother matching the artist string
        }

        if (matching(query, get_song_artist(id), match_diff)) {
          results.push_back(id);
        }
      }
    }
  }

  return results;
}

// urlencode - Source:
// https://gist.github.com/litefeel/1197e5c24eb9ec93d771

void hexchar(unsigned char c, unsigned char &hex1, unsigned char &hex2) {
  hex1 = c / 16;
  hex2 = c % 16;
  hex1 += hex1 <= 9 ? '0' : 'a' - 10;
  hex2 += hex2 <= 9 ? '0' : 'a' - 10;
}

std::string urlencode(std::string s) {
  const char *str = s.c_str();
  std::vector<char> v(s.size());
  v.clear();
  for (size_t i = 0, l = s.size(); i < l; i++) {
    char c = str[i];
    if ((c >= '0' && c <= '9') ||
      (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      c == '-' || c == '_' || c == '.' || c == '!' || c == '~' ||
      c == '*' || c == '\'' || c == '(' || c == ')') {
      v.push_back(c);
    }
    else if (c == ' ') {
      v.push_back('+');
    }
    else {
      v.push_back('%');
      unsigned char d1, d2;
      hexchar(c, d1, d2);
      v.push_back(d1);
      v.push_back(d2);
    }
  }

  return std::string(v.cbegin(), v.cend());
}

// =====================================================

bool _download_song_data_from_query(int new_id, const std::string& query) {
  auto yt_dlp_path = base_path_external_prog + "yt-dlp/";
  auto new_base = base_music_path_data + std::to_string(new_id);
  auto yt_dlp_args = " -I 1 \"https://music.youtube.com/search?q=" + query + "\" -xciw -f \"bestaudio/best\" --audio-format mp3 --audio-quality 0 --print-to-file \"%(artist)s\" " + new_base + ".artist --print-to-file \"%(track)s\" " + new_base + ".title -o \"" + new_base + "\".mp3";

  #ifdef __linux__
  return system((yt_dlp_path + "yt-dlp_linux" + yt_dlp_args).c_str()) == 0;
  #endif
  #ifdef __MINGW32__
  return system((yt_dlp_path + "yt-dlp.exe" + yt_dlp_args).c_str()) == 0;
  #endif
  #ifdef __APPLE__
  return system((yt_dlp_path + "yt-dlp_macos" + yt_dlp_args).c_str()) == 0;
  #endif

  return false;
}

bool _resize_cover_art(const std::string& temp_file, const std::string& output, int target_w, int target_h) {
  progress_bar_amount += 1.f; // Done downloading / started resizing the cover art image

  std::cout << "Info: Resizing cover art image." << std::endl;
  int w, h, channels;
  unsigned char* data = stbi_load(temp_file.c_str(), &w, &h, &channels, 0);

  if (!data) {
    std::cerr << "Error: Failed to decode image from " << temp_file << std::endl;
    return false;
  }

  std::vector<unsigned char> resized(target_w * target_h * channels);

  stbir_pixel_layout layout;
  switch (channels) {
    case 1: layout = STBIR_1CHANNEL; break;
    case 2: layout = STBIR_2CHANNEL; break;
    case 3: layout = STBIR_RGB; break;
    case 4: layout = STBIR_RGBA; break;
    default:
      std::cerr << "Error: Unsupported channel count for cover art image." << std::endl;
      stbi_image_free(data);
      return false;
  }

  stbir_resize(
    data, w, h, 0,
    resized.data(), target_w, target_h, 0,
    layout,
    STBIR_TYPE_UINT8,
    STBIR_EDGE_CLAMP,
    STBIR_FILTER_DEFAULT
  );

  progress_bar_amount += 1.f; // Done resizing the cover art image

  if (!stbi_write_png(output.c_str(), target_w, target_h, channels, resized.data(), target_w * channels)) {
    std::cerr << "Error: Failed to write cover art image." << std::endl;
    stbi_image_free(data);
    return false;
  }

  stbi_image_free(data);

  std::cout << "Info: Resized cover art image" << std::endl;
  progress_bar_amount += 1.f; // Done writing cover art image

  return true;
}

bool _download_cover_art(int new_id) {
  std::string new_base = base_music_path_data + std::to_string(new_id);
  std::string temp_file = ".cover_art.png.tmp";

  std::string artist_string = get_song_artist(new_id);
  std::string title_string = get_song_title(new_id);

  std::string query = "/search/tracks?q=" + urlencode(title_string + " " + artist_string);

  httplib::Client cli("https://www.last.fm");
  if (auto res = cli.Get(query)) {
    std::cout << res->body << std::endl;
  }

  if (!_resize_cover_art(temp_file, new_base + ".png", 1000, 1000)) return false;
  if (!_resize_cover_art(temp_file, new_base + ".small.png", 100, 100)) return false;

  std::remove(temp_file.c_str());

  return true;
}

bool download_song_from_query(const std::string& query) {
  if (pause_main_input_handling) return false; // Exit if a download is ongoing

  pause_main_input_handling = true;

  std::cout << "Info: Attempting to download song from query '" << query << "'." << std::endl;

  // Set progress bar
  progress_bar_string = "Downloading song...";
  progress_bar_amount = 0.f;
  progress_bar_total = 9.f; // 3 in download_song_from_query
                            // 3 in _resize_cover_art (* 2)

  int max_id = -1;

  for (const auto& entry : std::filesystem::directory_iterator(base_music_path_data)) {
    int id = std::stoi(entry.path().stem().string());

    if (id > max_id)
      max_id = id;
  }

  if (max_id == -1) {
    return false;
  }

  auto new_id = max_id + 1;

  progress_bar_amount += 1.f; // Done with getting the max_id

  if (!_download_song_data_from_query(new_id, query)) return false;
  std::cout << "Info: Downloaded song file and metadata for query '" << query << "'." << std::endl;
  progress_bar_amount += 1.f; // Done with downloading song file from query
  if (!_download_cover_art(new_id)) return false;
  std::cout << "Info: Downloaded song cover art for id '" << new_id << "'." << std::endl;
  progress_bar_amount += 1.f; // Done with downloading song cover from query

  return true;
}
