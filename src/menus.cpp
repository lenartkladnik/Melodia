#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <memory>
#include "../external/lib/RoundedRectangleShape.hpp"
#include "include/data.hpp"
#include "include/menus.hpp"

sf::Clock cursor_clock;

void search_reset_cursor() {
  cursor_clock.restart();
  show_cursor = true;
}

void search_move_cursor_left() {
  if (cursor_pos > 0) {
    // Keep cursor solid while changing cursor pos
    search_reset_cursor();

    cursor_pos--;
  }
}

void search_move_cursor_right() {
  if (cursor_pos < search_string.size()) {
    // Keep cursor solid while changing cursor pos
    search_reset_cursor();

    cursor_pos++;
  }
}

void search_running(sf::Text& search_before_cursor, sf::Text& search_after_cursor) {
  if (cursor_clock.getElapsedTime() >= sf::milliseconds(500)) {
    cursor_clock.restart();
    show_cursor = !show_cursor;
  }

  search_before_cursor.setString(search_string.substr(0, cursor_pos));
  search_after_cursor.setString(search_string.substr(cursor_pos));
}

void search_input(char32_t input) {
  // Keep cursor solid while inputting
  search_reset_cursor();

  if (input < 32 || input > 126) {
    switch (input) {
      case 8:
        if (search_string.size() > 0) {
          cursor_pos--;
          search_string.erase(cursor_pos, 1);
        }
        break;
      }
    }
    else if ((int)search_string.size() < search_max_char) {
      search_string.insert(cursor_pos, char32_to_utf8(input));
      cursor_pos++;
    }
}

void search_focus(sf::Vector2f& pos, sf::Text& search_before_cursor, sf::Text& search_after_cursor) {
  if (search_active) { // Set cursor pos
    if (search_string.empty()) {
      cursor_pos = 0;
    }
    else {
      auto c_pos = round((pos.x - search_before_cursor.getPosition().x) / (search_before_cursor.getGlobalBounds().size.x + search_after_cursor.getGlobalBounds().size.x) * search_string.size());

      // Keep cursor solid while changing cursor pos
      search_reset_cursor();

      if (c_pos > search_string.size()) {
        cursor_pos = search_string.size();
      }
      else {
        cursor_pos = c_pos;
      }
    }
  }
  else {
    search_active = true;
    search_before_cursor.setFillColor(text_color);
    search_before_cursor.setString(search_string.substr(0, cursor_pos));
    search_after_cursor.setString(search_string.substr(cursor_pos));
  }
}

void search_unfocus(sf::Text& search_before_cursor, sf::Text& search_after_cursor) {
  search_active = false;
  show_cursor = false;

  if (search_string.size() == 0) {
    search_before_cursor.setFillColor(light_text_color);
    search_before_cursor.setString("Search");
  }
}

void search_draw_cursor(sf::RenderWindow& window, sf::Text& search_before_cursor, sf::RoundedRectangleShape& search_background) {
  sf::RectangleShape cursor({1.2, search_background.getGlobalBounds().size.y - 15.f});
  cursor.setPosition({search_before_cursor.getPosition().x + 2.f + search_before_cursor.getGlobalBounds().size.x, search_before_cursor.getPosition().y + 1.f});
  cursor.setFillColor(white_color);
  window.draw(cursor);
}

std::unique_ptr<StaticData> init_general(sf::RenderWindow& window, sf::Vector2f search_size, sf::Vector2f search_pos, sf::Font& default_font) {
  search_string = "";
  cursor_pos = 0;

  // Search
  sf::RoundedRectangleShape search_background(search_size, 20, main_n);
  search_background.setPosition(search_pos);
  search_background.setFillColor(dark_background_color);

  sf::RoundedRectangleShape search_shadow({search_background.getGlobalBounds().size.x + 5.f, search_background.getGlobalBounds().size.y + 5.f}, 20, main_n);
  search_shadow.setPosition({search_background.getPosition().x - 2.5f, search_background.getPosition().y + 3.f});
  search_shadow.setFillColor(dark_main_color);

  sf::Text search_before_cursor(default_font, "Search");
  search_before_cursor.setPosition({search_background.getPosition().x + 10.f, search_background.getPosition().y + 6.f});
  search_before_cursor.setCharacterSize(20);
  search_before_cursor.setFillColor(light_text_color);

  sf::Text search_after_cursor(default_font, "");
  search_after_cursor.setCharacterSize(20);
  search_after_cursor.setFillColor(text_color);

  auto cancel_search_tex = std::make_shared<sf::Texture>();
  if (!cancel_search_tex->loadFromFile(base_path_misc + "close.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "close.png'." << std::endl;
  }
  cancel_search_tex->setSmooth(true);

  sf::Sprite cancel_search(*cancel_search_tex);
  cancel_search.setPosition({search_background.getPosition().x + search_background.getGlobalBounds().size.x - cancel_search.getGlobalBounds().size.x - 14.f, search_background.getPosition().y + 9.f});

  new_click_event(click_events, [](MenuData& menu_data) {
    search_string = "";
    cursor_pos = 0;
  }, cancel_search.getGlobalBounds(), sf::Mouse::Button::Left);

  return std::make_unique<StaticData>(StaticData {
    search_background,
    search_shadow,
    search_before_cursor,
    search_after_cursor,
    cancel_search_tex,
    cancel_search
  });
}
