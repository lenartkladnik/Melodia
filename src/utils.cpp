#include <SFML/Graphics.hpp>
#include <string>
#include "include/data.hpp"
#include "include/components.hpp"
#include "include/storage_handler.hpp"

#ifdef _WIN32
  #define POPEN _popen
  #define PCLOSE _pclose
#else
  #define POPEN popen
  #define PCLOSE pclose
#endif

void debug_draw_bounds(sf::RenderWindow& window, sf::FloatRect bounds) {
  sf::RectangleShape rect;
  rect.setSize(bounds.size);
  rect.setPosition(bounds.position);
  rect.setFillColor(sf::Color(255, 0, 0, 128));

  window.draw(rect);
}


void set_window(sf::State state) {
  is_fullscreen = state == sf::State::Fullscreen;
  window.create(sf::VideoMode(window_base_size), "Melodia", sf::Style::Default, state, window_settings);
}

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
  std::unique_ptr<FILE, int(*)(FILE*)> pipe(POPEN(cmd, "r"), PCLOSE);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

// ==============================================================================

std::shared_ptr<sf::Texture> load_texture(std::string name) {
  auto tex = std::make_shared<sf::Texture>();
  if (!tex->loadFromFile(base_path_misc + name)) {
    std::cerr << "Error: Failed to load '" << base_path_misc << name << "'." << std::endl;
  }
  tex->setSmooth(true);
  return tex;
}

// Adapted from: https://en.cppreference.com/cpp/string/byte/isalnum
bool is_string_valid_name(const std::string& s) {
  if (s.empty()) return false;

  return std::count_if(s.begin(), s.end(),
    [](unsigned char c){ return !(std::isalnum(c) || c == ' ' || c == '_') ;}
  ) == 0;
}

void mkdir(std::string path) {
  if (!std::filesystem::exists(path))
    std::filesystem::create_directory(path);
}

bool must_exist(std::string path) {
  if (!std::filesystem::exists(path)) {
    std::cout << "Error: Path not found '" << path << "'\n";
    return false;
  }
  return true;
}

std::string get_stem(std::string path) {
  return std::filesystem::path(path).stem();
}

std::string insert_in_stem(std::string path, std::string s) {
  size_t insert_at = path.size();

  auto last_dot = path.find_last_of('.');
  if (last_dot != std::string::npos && last_dot > path.find_last_of('/')) { // Has extension
    insert_at = last_dot;
  }

  return path.insert(insert_at, s);
}

std::string get_next_available_path(std::string path) {
  if (!std::filesystem::exists(path))
    return path;

  int i = 1;
  std::string new_path;
  do {
    new_path = insert_in_stem(path, path_counter_prefix + std::to_string(i));
    i++;
  } while (std::filesystem::exists(new_path));

  return new_path;
}

// Adapted from https://www.sfml-dev.org/tutorials/3.1/migration/sfml-3.0/#character-positions-in-text
sf::Vector2f find_character_pos(const sf::Text& text, size_t index) {
  const auto& glyphs = text.getShapedGlyphs();
  if (glyphs.empty())
    return text.getTransform().transformPoint({});

  for (const auto& g : glyphs) {
    if (g.cluster == index)
      return text.getTransform().transformPoint(g.position);
  }

  return {text.getPosition().x + text.getGlobalBounds().size.x, text.getPosition().y + text.getGlobalBounds().size.y}; // Very end of the string
}

sf::Vector2f find_character_size(const sf::Text& text, size_t index) {
  const auto& glyphs = text.getShapedGlyphs();
  if (glyphs.empty())
    return {0.f, 0.f};

  float width = 0.f;
  float height = 0.f;
  for (const auto& g : glyphs) {
    if (g.cluster == index) {
      width += g.glyph.bounds.size.x;
      if (g.glyph.bounds.size.y > height)
        height = g.glyph.bounds.size.y;
    }
  }

  return {width, height};
}

size_t find_character_at_pos_x(const std::string& string, const sf::Text& text, float pos_x) {
  size_t char_pos = string.size(); // This will be returned if all of the glyphs are behind pos_x
  for (size_t i = 0; i < string.size(); i++) {
    if (find_character_pos(text, i).x + (find_character_size(text, i).x / 2) > pos_x) {
      char_pos = i;
      break;
    }
  }
  return char_pos;
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

float getFontOffsetPixels(float target_size) {
  // This maps approximately what the results of font.getLineSpacing(*_font_size) for the
  // Inter font at certain font sizes would be. (Because that font looks normal in the UI)

  // This is a linear function fitted for these points:
  // (32.4, 21.5)
  // (36.0, 23.5)
  // (39.6, 26.5)
  // (43.2, 28.5)
  auto normal_line_spacing = 0.666667f * target_size - 0.2;

  return default_font.getLineSpacing(target_size) - normal_line_spacing;
}

void setFontSize(sf::Text& text, float target_size, unsigned int raster_mul) {
  auto raster_size = target_size * raster_mul;
  float scale = 1.0f / raster_mul;

  text.setCharacterSize((unsigned int)raster_size);
  text.setScale({scale, scale});

  auto offset = getFontOffsetPixels(target_size);
  text.setOrigin({0, offset});
}

void reset_globals() {
  // All of the std::vector objects that get cleared here contain
  // some pointers to objects in memory that has changed

  search_res_click_events.clear();
  search_results.clear();

  click_events.clear();

  popup_components.clear();

  text_events.clear();

  kb_events.clear();

  focus_events.clear();

  scroll_events.clear();

  // Reset the global z-index since
  // all the objects must be redrawn
  global_z_index = 0;
}
