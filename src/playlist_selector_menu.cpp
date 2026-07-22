#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <thread>
#include "../external/lib/RoundedRectangleShape.hpp"
#include "include/data.hpp"
#include "include/player_menu.hpp"
#include "include/download.hpp"
#include "include/utils.hpp"
#include "include/song_containers.hpp"

std::shared_ptr<StaticPlaylistSelectorData> init_playlist_selector(sf::RenderWindow& window) {
  reset_globals();

  auto trash_input_c_tex = load_texture("download.png");

  float search_size_x = 600.f;
  auto search = std::make_shared<InputComponent>(
    window,
    "playlist_search_input_c", // id
    sf::Vector2f{search_size_x, 40.f}, // size
    sf::Vector2f{window_size.x / 2 - search_size_x / 2, 12.f}, // position
    "Search",
    trash_input_c_tex,
    download_from_search
  );

  auto playlists = get_all_playlists();

  DTCache drawables_cache;


  auto data = std::make_shared<StaticPlaylistSelectorData>();
  data->search = search;
  data->playlists = playlists;
  data->drawables_cache = drawables_cache;
  return data;
}

bool display_playlist_selector(MenuData::PlaylistSelectorData& playlist_sel, sf::RenderWindow& window, MenuData& menu_data) {
  global_z_index = 0;

  auto& data = *playlist_sel.data;

  window.clear(main_color);

  // Drag and drop area  TODO: Make this scrollable
  sf::Vector2f playlist_drop_area_gap(60.f, 180.f);
  sf::RoundedRectangleShape playlist_drop_area_background(
    sf::Vector2f(
      window_size.x - playlist_drop_area_gap.x * 2,
      window_size.y - playlist_drop_area_gap.y - 20.f
    ),
    8,
    main_n
  );
  playlist_drop_area_background.setPosition(playlist_drop_area_gap);
  playlist_drop_area_background.setFillColor(lighter_background_color);

  AreaComponent playlist_drop_area(
    "playlist_drop_area",
    playlist_drop_area_background.getGlobalBounds(),
    [](MenuData&){}
  );

  window.draw(playlist_drop_area_background);

  // Favourites
  // TODO: Implement

  // Playlists

  float selector_gap = 20.f;
  float cover_offset = 10.f;
  auto total_playlist_sel_size = (selector_cover_size + selector_size.x + selector_gap);
  int max_playlists_per_line = (int)(window_size.x / total_playlist_sel_size);
  float padding_to_center = (window_size.x - (total_playlist_sel_size * max_playlists_per_line)) / 2;

  // Playlist Items

  for (size_t i = 0; i < data.playlists.size(); i++) {
    sf::Vector2f cover_pos = {
      total_playlist_sel_size * (i % max_playlists_per_line) + padding_to_center + (cover_offset / 2),
      (selector_cover_size + selector_gap) * ((int)(i / max_playlists_per_line) + 1) + (cover_offset / 2)
    };

    auto cover = std::make_shared<sf::RoundedRectangleShape>(sf::Vector2f(selector_cover_size - cover_offset, selector_cover_size - cover_offset), 8, main_n);

    cover->setPosition(cover_pos);

    if (data.drawables_cache.contains(i)) { // Only draw if the cache has it

      auto mouse_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

      auto& cover_dt = data.drawables_cache.get_item(i, data.drawables_cache.name_to_z_index(i, "cover"));
      if (cover_dt.drawformable->getGlobalBounds().contains(mouse_pos)) {
        // TODO: Hover effect
      }

      data.drawables_cache.draw(i, window);

      new_click_event(click_events, "playlist_play_" + std::to_string(i), [i](MenuData& menu_data) {
        switch_to_player(menu_data, std::get<MenuData::PlaylistSelectorData>(menu_data.data).data->playlists[i]);
      }, cover->getGlobalBounds(), sf::Mouse::Button::Left);

      if (!pause_main_input_handling) {
        // Hover checks

        if (cover->getGlobalBounds().contains(mouse_pos)) {
          window.setMouseCursor(hand_cursor);

          playlist_sel.reset_cursor = false;
        }

        continue;
      }
    }

    auto cover_texture = std::make_shared<sf::Texture>();
    if (!cover_texture->loadFromFile(base_music_path_playlists + data.playlists[i] + ".png")) {
      std::cerr << "Error: Failed to load '" << base_music_path_playlists << data.playlists[i] << ".png" << "'." << std::endl;
    }
    cover_texture->setSmooth(true);

    cover->setTexture(cover_texture.get());

    auto sel_background = std::make_shared<sf::RoundedRectangleShape>(sf::Vector2f(selector_cover_size + selector_size.x, selector_size.y), 8, main_n);
    sel_background->setPosition({cover->getPosition().x - (cover_offset / 2), cover->getPosition().y - (cover_offset / 2)});
    sel_background->setFillColor(background_shadow_color);

    auto sel_background_shadow = std::make_shared<sf::RoundedRectangleShape>(sel_background->getGlobalBounds().size, 8, main_n);
    sel_background_shadow->setPosition({sel_background->getPosition().x + shadow_offset, sel_background->getPosition().y + shadow_offset});
    sel_background_shadow->setFillColor(background_shadow_color_transparent);

    auto playlist_name = std::make_shared<sf::Text>(default_font, data.playlists[i]);
    playlist_name->setFillColor(text_color);
    setFontSize(*playlist_name, large_font_size);
    playlist_name->setPosition({
      sel_background->getPosition().x + selector_cover_size + 5.f,
      sel_background->getPosition().y + 10.f
    });

    auto playlist_size = std::make_shared<sf::Text>(default_font, "Items: " + std::to_string(get_playlist(data.playlists[i]).size()));
    playlist_size->setFillColor(light_text_color);
    setFontSize(*playlist_size, small_font_size);
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


  // Search Results

  if (playlist_sel.data->search->is_active() && playlist_sel.data->search->is_focused()) {
    search_was_active = true;

    playlist_sel.data->search->background_set_corner_radii(std::array<float, 4>{
      playlist_sel.data->search->background_get_corner_radius(0),
      playlist_sel.data->search->background_get_corner_radius(1),
      0.f,
      0.f
    });

    can_search_string_scroll = true;

    if (playlist_sel.data->search->should_input_refresh()) {
      search_results = search_all_songs(playlist_sel.data->search->get_input_string());
      playlist_sel.data->search->input_refresh();
      search_res_click_events.clear();
    }

    float search_results_background_h = std::min((playlist_search_entry_unit) * (search_results.size()), (float)window_size.y - playlist_search_entry_unit);
    sf::RoundedRectangleShape search_results_background({data.search->background_bounds().size.x, search_results_background_h + 10.f}, 8, main_n);
    search_results_background.setPosition({data.search->background_pos().x, data.search->background_pos().y + data.search->background_bounds().size.y});
    search_results_background.setFillColor(light_background_color);
    search_results_background.setCornerRadii(std::array<float, 4>{0.f, 0.f, search_results_background.getCornersRadius(2), search_results_background.getCornersRadius(3)});

    AreaComponent search_res_area(
      "search_res_area",
      search_results_background.getGlobalBounds(),
      [](MenuData& menu_data){
        if (search_was_active)
          std::get<MenuData::PlaylistSelector>(menu_data.data).data->search->focus({-1, -1}); // {-1, -1} since the position won't be changed anyway
      },
      false
    );

    new_scroll_event(scroll_events, "search_results_background", search_results_background.getGlobalBounds(), playlist_sel_scroll, can_search_string_scroll);


    window.draw(search_results_background);

    // Show search results

    int idx = 0;
    float last_y_pos = 0.f;

    float total_content_h = (playlist_search_entry_height + 10.f) * search_results.size() + 20.f;
    float max_scroll = std::max(playlist_search_scroll_lower_bound / 2, total_content_h - search_results_background_h - playlist_search_entry_unit / 2 - 10.f);
    playlist_sel_scroll = std::clamp(playlist_sel_scroll, playlist_search_scroll_lower_bound, max_scroll);

    float view_left = search_results_background.getPosition().x / window_size.x;
    float view_top = (search_results_background.getPosition().y) / window_size.y;
    float view_width = search_results_background.getGlobalBounds().size.x / window_size.x;
    float view_height = search_results_background_h / window_size.y;

    float download_prompt_height = 75.f;

    sf::View search_results_view;
    search_results_view.setSize({
      search_results_background.getGlobalBounds().size.x,
      search_results_background_h
    });
    search_results_view.setCenter({
      search_results_background.getPosition().x + search_results_background.getGlobalBounds().size.x / 2.f,
      search_results_background.getPosition().y + search_results_background_h / 2.f + playlist_sel_scroll
    });
    search_results_view.setViewport(sf::FloatRect(
      {view_left, view_top},
      {view_width, view_height}
    ));

    window.setView(search_results_view);

    for (const int& search_res_id : search_results) {
      last_y_pos = (playlist_search_entry_height + 10.f) * idx + 10.f;

      auto search_result = create_small_song_container(
        search_res_id,
        sf::Vector2f(
          search_results_background.getPosition().x + 10.f,
          last_y_pos
        ),
        sf::Vector2f(
          search_results_background.getGlobalBounds().size.x - 10.f,
          playlist_search_entry_height + 5.f
        )
      );
      draw_small_song_container(search_result);

      auto search_res_more_bounds = search_result->more.value()->getGlobalBounds();
      search_res_more_bounds.size.y = 30.f;
      search_res_more_bounds.position.y -= 15.f;

      auto actual_results_bounds = search_results_background.getGlobalBounds();
      auto result_bounds_offset = 40.f;
      actual_results_bounds.position.y += playlist_sel_scroll - result_bounds_offset;
      actual_results_bounds.size.y += result_bounds_offset;

      if (actual_results_bounds.contains(search_res_more_bounds.position)) {
        new_click_event(search_res_click_events, "search_res_more_bounds_" + std::to_string(search_res_id), [search_res_id](MenuData& menu_data) {
          std::cout << "Edit " << search_res_id << std::endl;
        }, search_res_more_bounds, sf::Mouse::Button::Left, nullptr, search_results_view);
      }

      auto unit_size = search_result->background.getGlobalBounds().size.y + 13.5f;
      last_y_pos += unit_size;

      idx++;
    }

    window.setView(default_view);
  }
  else {
    search_was_active = false;
    can_search_string_scroll = false;
    data.search->draw_input_shadow();
    data.search->background_reset_corner_radii();
  }

  data.search->draw();


  // Popups / Overlays

  // Progress bar for downloading
  if (!progress_bar_string.empty()) {
    sf::RoundedRectangleShape pbar_background({550.f, 150.f}, 8, main_n);
    pbar_background.setFillColor(background_color);
    pbar_background.setPosition({
      (window_size.x / 2) - (pbar_background.getGlobalBounds().size.x / 2),
      (window_size.y / 2) - (pbar_background.getGlobalBounds().size.y / 2)
    });

    sf::Text pbar_text(default_font, progress_bar_string);
    setFontSize(pbar_text, medium_font_size);
    pbar_text.setFillColor(text_color);
    pbar_text.setPosition({
      pbar_background.getPosition().x + 5.f,
      pbar_background.getPosition().y + 5.f
    });

    sf::Text pbar_doing_text(default_font, progress_bar_doing_string);
    setFontSize(pbar_doing_text, small_font_size);
    pbar_doing_text.setFillColor(light_text_color);
    pbar_doing_text.setPosition({
      pbar_background.getPosition().x + (pbar_background.getGlobalBounds().size.x / 2) - (pbar_doing_text.getGlobalBounds().size.x / 2),
      pbar_background.getPosition().y + pbar_background.getGlobalBounds().size.y - pbar_doing_text.getGlobalBounds().size.y - 5.f
    });

    sf::RoundedRectangleShape pbar_progress({pbar_background.getGlobalBounds().size.x - 10.f, progress_height}, progress_round, progress_n);
    pbar_progress.setFillColor(progress_color);
    pbar_progress.setPosition({pbar_text.getPosition().x, pbar_text.getPosition().y + pbar_text.getGlobalBounds().size.y + 20.f});

    auto done = progress_bar_amount / progress_bar_total;

    sf::RoundedRectangleShape pbar_progress_done({done * pbar_progress.getGlobalBounds().size.x, pbar_progress.getGlobalBounds().size.y}, progress_round, progress_n);
    pbar_progress_done.setFillColor(progress_done_color);
    pbar_progress_done.setPosition(pbar_progress.getPosition());

    window.draw(pbar_background);
    window.draw(pbar_text);
    window.draw(pbar_doing_text);
    window.draw(pbar_progress);
    window.draw(pbar_progress_done);
  }

  window.display();

  return true;
}

void switch_to_playlist_selector(MenuData& menu_data, sf::RenderWindow& window) {
  menu_data.data = MenuData::PlaylistSelectorData();
  menu_data.type = MenuData::PlaylistSelector;

  input_max_char = playlist_search_max_char;

  std::get<MenuData::PlaylistSelector>(menu_data.data).data = init_playlist_selector(window);
  std::get<MenuData::PlaylistSelector>(menu_data.data).is_valid = true;

  if (!std::get<MenuData::PlaylistSelectorData>(menu_data.data).is_valid || !std::holds_alternative<MenuData::PlaylistSelectorData>(menu_data.data)) {
    std::cerr << "Failed to load menu... TODO: Fallback" << std::endl;
  }
}
