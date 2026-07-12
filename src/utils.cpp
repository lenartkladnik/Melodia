#include <SFML/Graphics.hpp>
#include <string>
#include "include/data.hpp"

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
