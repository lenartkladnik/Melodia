#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "RoundedRectangleShape.hpp"
#include "data.hpp"
#include "menus.hpp"
#include "player_menu.hpp"

std::unique_ptr<StaticPlaylistSelectorData> init_playlist_selector(sf::RenderWindow& window, sf::Font& default_font) {
  float search_size_x = 600.f;
  auto general = init_general(
    window,
    {search_size_x, 40.f},
    {window.getSize().x / 2 - search_size_x / 2, 12.f},
    default_font
  );

  // Playlist play overlay
  auto playlist_play_tex = std::make_shared<sf::Texture>();
  if (!playlist_play_tex->loadFromFile(base_path_misc + "queue_play.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "queue_play.png'." << std::endl;
  }
  playlist_play_tex->setSmooth(true);

  sf::Sprite playlist_play(*playlist_play_tex);
  auto playlist_play_scale = (selector_cover_size - 10.f) / playlist_play.getGlobalBounds().size.x;
  playlist_play.setScale({playlist_play_scale, playlist_play_scale});

  // The controls in the upper rightish corner of the screen

  sf::RoundedRectangleShape control_corner({55.f, 50.f}, out_round, main_n);
  control_corner.setPosition({window_size.x - control_corner.getGlobalBounds().size.x - 20.f, -10.f});
  control_corner.setFillColor(background_color);

  sf::RoundedRectangleShape control_corner_shadow(control_corner.getGlobalBounds().size, out_round, main_n);
  control_corner_shadow.setPosition({control_corner.getPosition().x + shadow_offset, control_corner.getPosition().y + shadow_offset});
  control_corner_shadow.setFillColor(dark_main_color);

  // Items in the control corner

  auto add_playlist_tex = std::make_shared<sf::Texture>();
  if (!add_playlist_tex->loadFromFile(base_path_misc + "plus.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "plus.png'." << std::endl;
  }
  add_playlist_tex->setSmooth(true);

  sf::Sprite add_playlist(*add_playlist_tex);
  add_playlist.setPosition({control_corner.getPosition().x + control_corner_gap, control_corner.getPosition().y + 12.f});


  auto playlists = get_all_playlists();

  DTCache drawables_cache;

  return std::make_unique<StaticPlaylistSelectorData>(StaticPlaylistSelectorData {
    general->search_background,
    general->search_shadow,
    general->search_before_cursor,
    general->search_after_cursor,
    general->cancel_search_tex,
    general->cancel_search,
    control_corner,
    control_corner_shadow,
    add_playlist_tex,
    add_playlist,
    playlist_play_tex,
    playlist_play,
    playlists,
    drawables_cache,
  });
}

bool display_playlist_selector(MenuData::PlaylistSelectorData& playlist_sel, sf::RenderWindow& window, MenuData& menu_data, sf::Font& default_font) {
  auto& data = *playlist_sel.data;

  window.clear(main_color);

  window.draw(data.search_shadow);
  window.draw(data.search_background);
  window.draw(data.search_before_cursor);
  window.draw(data.search_after_cursor);
  window.draw(data.cancel_search);
  if (show_cursor) {
    search_draw_cursor(window, data.search_before_cursor, data.search_background);
  }

  // Playlists

  float selector_gap = 20.f;
  float cover_offset = 10.f;
  auto total_playlist_sel_size = (selector_cover_size + selector_size.x + selector_gap);
  int max_playlists_per_line = (int)(window.getSize().x / total_playlist_sel_size);
  float padding_to_center = (window_base_size.x - (total_playlist_sel_size * max_playlists_per_line)) / 2;

  for (size_t i = 0; i < data.playlists.size(); i++) {
    sf::Vector2f cover_pos = {
      total_playlist_sel_size * (i % max_playlists_per_line) + padding_to_center + (cover_offset / 2),
      (selector_cover_size + selector_gap) * ((int)(i / max_playlists_per_line) + 1) + (cover_offset / 2)
    };

    data.playlist_play.setPosition(cover_pos);
    if (data.drawables_cache.contains(i)) { // Only draw existing ones

      // Hover checks

      auto mouse_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
      bool draw_playlist_play = false;

      auto& cover_dt = data.drawables_cache.get_item(i, data.drawables_cache.name_to_z_index(i, "cover"));
      if (cover_dt.drawformable->getGlobalBounds().contains(mouse_pos)) {
        draw_playlist_play = true;
      }

      data.drawables_cache.draw(i, window);

      if (draw_playlist_play) {
        window.draw(data.playlist_play);
      }

      if (
        (data.playlist_play.getGlobalBounds().contains(mouse_pos) && draw_playlist_play)
      ) {
        window.setMouseCursor(hand_cursor);

        playlist_sel.reset_cursor = false;
      }


      // Click checks

      if (!held_left_mb_down && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        playlist_sel.playlist_play_pos = mouse_pos;

      if (draw_playlist_play && playlist_sel.playlist_play_pos.x != -1 && data.playlist_play.getGlobalBounds().contains(playlist_sel.playlist_play_pos)) { // Clicked on the play / playlist art image
        if (get_playlist(data.playlists[i]).size() > 0)
          switch_to_player(menu_data, data.playlists[i], default_font);
        return false;
      }

      held_left_mb_down = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

      continue;
    }

    auto cover = std::make_shared<sf::RoundedRectangleShape>(sf::Vector2f(selector_cover_size - cover_offset, selector_cover_size - cover_offset), 8, main_n);

    auto cover_texture = std::make_shared<sf::Texture>();
    if (!cover_texture->loadFromFile(base_music_path_playlists + data.playlists[i] + ".png")) {
      std::cerr << "Error: Failed to load '" << base_music_path_playlists << data.playlists[i] << ".png" << "'." << std::endl;
    }
    cover_texture->setSmooth(true);

    cover->setTexture(cover_texture.get());
    cover->setPosition(cover_pos);

    auto sel_background = std::make_shared<sf::RoundedRectangleShape>(sf::Vector2f(selector_cover_size + selector_size.x, selector_size.y), 8, main_n);
    sel_background->setPosition({cover->getPosition().x - (cover_offset / 2), cover->getPosition().y - (cover_offset / 2)});
    sel_background->setFillColor(background_shadow_color);

    auto sel_background_shadow = std::make_shared<sf::RoundedRectangleShape>(sel_background->getGlobalBounds().size, 8, main_n);
    sel_background_shadow->setPosition({sel_background->getPosition().x + shadow_offset, sel_background->getPosition().y + shadow_offset});
    sel_background_shadow->setFillColor(dark_main_color);

    auto playlist_name = std::make_shared<sf::Text>(default_font, data.playlists[i]);
    playlist_name->setFillColor(text_color);
    playlist_name->setCharacterSize(24);
    playlist_name->setPosition({
      sel_background->getPosition().x + selector_cover_size + 5.f,
      sel_background->getPosition().y + 10.f
    });

    auto playlist_size = std::make_shared<sf::Text>(default_font, "Items: " + std::to_string(get_playlist(data.playlists[i]).size()));
    playlist_size->setFillColor(light_text_color);
    playlist_size->setCharacterSize(18);
    playlist_size->setPosition({
      sel_background->getPosition().x + sel_background->getGlobalBounds().size.x - playlist_size->getGlobalBounds().size.x - 20.f,
      sel_background->getPosition().y + sel_background->getGlobalBounds().size.y - playlist_size->getGlobalBounds().size.y - 15.f
    });

    data.drawables_cache.add(i, "sel_background_shadow", DTPair{std::make_shared<DrawformableObject>(sel_background_shadow, sel_background_shadow), nullptr});
    data.drawables_cache.add(i, "sel_background", DTPair{std::make_shared<DrawformableObject>(sel_background, sel_background), nullptr});
    data.drawables_cache.add(i, "playlist_name", DTPair{std::make_shared<DrawformableObject>(playlist_name, playlist_name), nullptr});
    data.drawables_cache.add(i, "playlist_size", DTPair{std::make_shared<DrawformableObject>(playlist_size, playlist_size), nullptr});
    data.drawables_cache.add(i, "cover", DTPair{std::make_shared<DrawformableObject>(cover, cover), cover_texture});
  }

  window.draw(data.control_corner_shadow);
  window.draw(data.control_corner);
  window.draw(data.add_playlist);

  window.display();

  return true;
}

void switch_to_playlist_selector(MenuData& menu_data, sf::RenderWindow& window, sf::Font& default_font) {
  menu_data.data = MenuData::PlaylistSelectorData();
  menu_data.type = MenuData::PlaylistSelector;

  // Change the maximum string length for search
  search_max_char = playlist_search_max_char;

  std::get<MenuData::PlaylistSelector>(menu_data.data).data = init_playlist_selector(window, default_font);
  std::get<MenuData::PlaylistSelector>(menu_data.data).is_valid = true;

  if (!std::get<MenuData::PlaylistSelectorData>(menu_data.data).is_valid || !std::holds_alternative<MenuData::PlaylistSelectorData>(menu_data.data)) {
    std::cerr << "Failed to load menu... TODO: Fallback" << std::endl;
  }
}
