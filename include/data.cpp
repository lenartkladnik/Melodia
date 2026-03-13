#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <random>
#include <cctype>

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
extern const std::string base_data_path = ".music_data/";
extern const int queue_items = 6;
extern const float move_speed = 20.f;
extern const float match_diff = 3.f;
extern const int queue_search_max_char = 28;
extern const int queue_max_char = 26;
extern const float queue_contracted_width = 50.f;

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

std::random_device rd;
std::mt19937 rand_generator(rd());

std::vector<int> get_playlist(const std::string& name) {
  std::vector<int> ids;
  std::string entry;
  std::ifstream playlist_file(base_data_path + "playlists/" + name);

  while (std::getline(playlist_file, entry)) {
    int id = std::stoi(entry);

    if (std::find(ids.begin(), ids.end(), id) == ids.end())
      ids.push_back(id);
  }

  std::shuffle(ids.begin(), ids.end(), rand_generator);

  return ids;
}

std::string construct_song_path(int id) {
  return base_data_path + "data/" + std::to_string(id);
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

bool matching(const std::string& s1, const std::string& s2, float diff, const char split = ' ') {
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
