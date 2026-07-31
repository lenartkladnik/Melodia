#include <SFML/Graphics.hpp>
#include <string>
#include "include/data.hpp"
#include "include/storage_handler.hpp"

void debug_draw_bounds(sf::RenderWindow& window, sf::FloatRect bounds) {
  sf::RectangleShape rect;
  rect.setSize(bounds.size);
  rect.setPosition(bounds.position);
  rect.setFillColor(sf::Color(255, 0, 0, 128));

  window.draw(rect);
}

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
