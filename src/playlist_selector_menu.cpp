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

std::shared_ptr<StaticPlaylistSelectorData> init_playlist_selector(sf::RenderWindow& window) {
  reset_globals();

  float search_size_x = 600.f;
  auto search = std::make_shared<InputComponent>(
    window,
    "playlist_search_input_c", // id
    sf::Vector2f{search_size_x, 40.f}, // size
    sf::Vector2f{window_size.x / 2 - search_size_x / 2, 12.f}, // position
    "Search"
  );
  // new_click_event(click_events, [](MenuData& menu_data) {
  //   std::get<MenuData::PlaylistSelector>(menu_data.data).data->search->clear_input();
  // }, search.cancel_input_bounds(), sf::Mouse::Button::Left);

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

  // Popups

  PopupComponent* add_playlist_popup = PopupComponent::Create("add_playlist");
  add_playlist_popup->hidden = true;
  add_playlist_popup->z_index = 1;

  sf::RoundedRectangleShape add_playlist_background({600.f, 300.f}, 8, main_n);
  add_playlist_background.setFillColor(background_color);
  add_playlist_background.setPosition({
    (window_size.x / 2) - (add_playlist_background.getGlobalBounds().size.x / 2),
    (window_size.y / 2) - (add_playlist_background.getGlobalBounds().size.y / 2)
  });

  sf::Vector2f save_button_size = {90.f, 40.f};
  auto save_button = std::make_shared<ButtonComponent>(
    window,
    "add_playlist_save_button", // id
    "Create",
    save_button_size, // size
    sf::Vector2f{ // position
      add_playlist_background.getPosition().x + add_playlist_background.getGlobalBounds().size.x - save_button_size.x - 10.f,
      add_playlist_background.getPosition().y + add_playlist_background.getGlobalBounds().size.y - save_button_size.y - 10.f
    },
    [](MenuData& menu_data){
      popup_components.at("add_playlist")->hidden = true;
    }
  );

  auto title_input = std::make_shared<InputComponent>(
    window,
    "add_playlist_title_input_c", // id
    sf::Vector2f{200.f, 38.f}, // size
    sf::Vector2f{ // position
      add_playlist_background.getPosition().x + 10.f,
      add_playlist_background.getPosition().y + 10.f
    },
    "Title"
  );

  add_playlist_popup->new_input("title_input", title_input);
  add_playlist_popup->new_button("save_button", save_button);
  add_playlist_popup->new_rounded_rectangle_shape("background", std::move(add_playlist_background));


  auto playlists = get_all_playlists();

  DTCache drawables_cache;


  auto data = std::make_shared<StaticPlaylistSelectorData>();
  data->search = search;
  data->control_corner = control_corner;
  data->control_corner_shadow = control_corner_shadow;
  data->add_playlist_tex = add_playlist_tex;
  data->add_playlist = add_playlist;
  data->playlist_play_tex = playlist_play_tex;
  data->playlist_play = playlist_play;
  data->playlists = playlists;
  data->drawables_cache = drawables_cache;
  return data;
}

bool display_playlist_selector(MenuData::PlaylistSelectorData& playlist_sel, sf::RenderWindow& window, MenuData& menu_data) {
  auto& data = *playlist_sel.data;

  window.clear(main_color);

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

    data.playlist_play->setPosition(cover_pos);
    if (data.drawables_cache.contains(i)) { // Only draw existing ones

      auto mouse_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
      bool draw_playlist_play = false;

      auto& cover_dt = data.drawables_cache.get_item(i, data.drawables_cache.name_to_z_index(i, "cover"));
      if (cover_dt.drawformable->getGlobalBounds().contains(mouse_pos)) {
        draw_playlist_play = true;
      }

      data.drawables_cache.draw(i, window);

      new_click_event(click_events, "playlist_play_" + std::to_string(i), [i](MenuData& menu_data) {
        switch_to_player(menu_data, std::get<MenuData::PlaylistSelectorData>(menu_data.data).data->playlists[i]);
      }, data.playlist_play->getGlobalBounds(), sf::Mouse::Button::Left);

      if (!pause_main_input_handling) {
        if (draw_playlist_play) {
          window.draw(*data.playlist_play);
        }

        // Hover checks

        // TODO: Figure out how to only change hover state when the obj has hidden false
        if (data.playlist_play->getGlobalBounds().contains(mouse_pos) && draw_playlist_play) {
          window.setMouseCursor(hand_cursor);

          playlist_sel.reset_cursor = false;
        }

        continue;
      }
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

  if (playlist_sel.data->search->is_active()) {
    can_search_string_scroll = true;

    if (playlist_sel.data->search->should_input_refresh()) {
      search_results = search_all_songs(playlist_sel.data->search->get_input_string());
      playlist_sel.data->search->input_refresh();
      search_res_click_events = {};
    }

    auto entry_h = queue_cover_size + 10.f;

    auto approx_unit = entry_h + 10.f;
    auto approx_leftover = search_results.empty() ? 5.f : 13.f;
    auto approx_dl_prompt_h = (approx_unit - 60.f);
    float search_results_background_h = std::min((approx_unit) * (search_results.size() + 1) - approx_leftover, (float)window_size.y - approx_unit - approx_dl_prompt_h);
    sf::RoundedRectangleShape search_results_background({data.search->background_bounds().size.x, search_results_background_h + approx_dl_prompt_h / 2}, 8, main_n);
    search_results_background.setPosition({data.search->background_pos().x, data.search->background_pos().y + data.search->background_bounds().size.y - 20.f});
    search_results_background.setFillColor(dark_background_color);

    window.draw(search_results_background);

    new_scroll_event(scroll_events, "search_results_background", search_results_background.getGlobalBounds(), playlist_sel_scroll, can_search_string_scroll);

    // Show search results

    int idx = 0;
    float last_y_pos = 0.f;

    float total_content_h = (entry_h + 10.f) * search_results.size() + 20.f;
    float max_scroll = std::max(0.f, total_content_h - search_results_background_h + approx_unit / 2 + approx_dl_prompt_h / 2);
    playlist_sel_scroll = std::clamp(playlist_sel_scroll, 0.f, max_scroll);

    float view_left = search_results_background.getPosition().x / window_size.x;
    float view_top = (search_results_background.getPosition().y) / window_size.y;
    float view_width = search_results_background.getGlobalBounds().size.x / window_size.x;
    float view_height = search_results_background_h / window_size.y;

    sf::View search_results_view;
    search_results_view.setSize({
      search_results_background.getGlobalBounds().size.x,
      search_results_background_h
    });
    search_results_view.setCenter({
      search_results_background.getPosition().x + search_results_background.getGlobalBounds().size.x / 2.f,
      search_results_background.getPosition().y - approx_dl_prompt_h + search_results_background_h / 2.f + playlist_sel_scroll
    });
    search_results_view.setViewport(sf::FloatRect(
      {view_left, view_top},
      {view_width, view_height}
    ));

    window.setView(search_results_view);

    for (const int& search_res_id : search_results) {
      std::string search_res_path = base_music_path_data + std::to_string(search_res_id);

      last_y_pos = (entry_h + 10.f) * idx + 10.f;

      sf::Texture search_res_cover_texture(search_res_path + ".small.png");
      sf::RoundedRectangleShape search_res_cover({queue_cover_size, queue_cover_size}, 8, main_n);
      search_res_cover.setTexture(&search_res_cover_texture);
      search_res_cover.setPosition({search_results_background.getPosition().x + 10.f, last_y_pos});

      sf::RoundedRectangleShape search_res_cover_shadow({queue_cover_size, queue_cover_size}, 8, main_n);
      search_res_cover_shadow.setFillColor(dark_background_shadow_color);
      search_res_cover_shadow.setPosition({search_res_cover.getPosition().x + small_shadow_offset, search_res_cover.getPosition().y + small_shadow_offset});

      sf::Text search_res_title(default_font, get_song_title(search_res_id));
      search_res_title.setFillColor(title_color);
      setFontSize(search_res_title, small_font_size);
      search_res_title.setPosition({
        search_res_cover.getPosition().x + search_res_cover.getGlobalBounds().size.x + 15.f,
        search_res_cover.getPosition().y + queue_cover_size / 3 - 10.f
      });

      sf::Text search_res_artist(default_font, get_song_artist(search_res_id));
      search_res_artist.setFillColor(artist_color);
      setFontSize(search_res_artist, small_font_size);
      search_res_artist.setPosition({search_res_title.getPosition().x, search_res_title.getPosition().y + 20.f});

      sf::RoundedRectangleShape search_res_background({search_results_background.getGlobalBounds().size.x - 10.f, entry_h + 5.f}, 8, main_n);
      search_res_background.setFillColor(background_shadow_color);
      search_res_background.setPosition({search_res_cover.getPosition().x - 5.f, search_res_cover.getPosition().y - 5.f});

      sf::Text search_res_more(default_font, "...");
      search_res_more.setFillColor(text_color);
      setFontSize(search_res_more, small_font_size);
      search_res_more.setPosition({
        search_res_background.getPosition().x + search_res_background.getGlobalBounds().size.x - search_res_more.getGlobalBounds().size.x - 20.f,
        (search_res_background.getPosition().y - 5.f) + (search_res_background.getGlobalBounds().size.y - 5.f) / 2 - search_res_more.getGlobalBounds().size.y
      });

      window.draw(search_res_background);
      window.draw(search_res_cover_shadow);
      window.draw(search_res_cover);
      window.draw(search_res_title);
      window.draw(search_res_artist);
      window.draw(search_res_more);

      auto search_res_more_bounds = search_res_more.getGlobalBounds();
      search_res_more_bounds.size.y = 30;
      search_res_more_bounds.position.y += 45.f - playlist_sel_scroll;

      if (search_results_background.getGlobalBounds().contains(search_res_more_bounds.position)) {
        new_click_event(search_res_click_events, "search_res_more_bounds", [search_res_id](MenuData& menu_data) {
          std::cout << "Edit " << search_res_id << std::endl;
        }, search_res_more_bounds, sf::Mouse::Button::Left, nullptr, search_results_view);
      }

      auto unit_size = search_res_background.getGlobalBounds().size.y + 13.5;
      last_y_pos += unit_size;

      idx++;
    }

    window.setView(window.getDefaultView());

    // Download prompt

    sf::RoundedRectangleShape download_prompt_background({search_results_background.getGlobalBounds().size.x - 10.f, 75.f}, 8, main_n);
    download_prompt_background.setFillColor(background_shadow_color);
    download_prompt_background.setPosition({
      search_results_background.getPosition().x + 5.f,
      search_results_background.getPosition().y + search_results_background.getGlobalBounds().size.y - download_prompt_background.getGlobalBounds().size.y - 5.f
    });

    sf::Text download_prompt(default_font, "Download '" + playlist_sel.data->search->get_input_string() + "'");
    download_prompt.setPosition({
      download_prompt_background.getPosition().x + 5.f,
      download_prompt_background.getPosition().y + download_prompt_background.getGlobalBounds().size.y / 2 - download_prompt.getGlobalBounds().size.y / 2
    });
    setFontSize(download_prompt, small_font_size);
    download_prompt.setFillColor(light_text_color);

    window.draw(download_prompt_background);
    window.draw(download_prompt);

    new_click_event(search_res_click_events, "download_prompt_background", [](MenuData& menu_data) {
      download_song_thread = std::unique_ptr<std::thread>(new std::thread(
        download_song_from_query,
        std::get<MenuData::PlaylistSelector>(menu_data.data).data->search->get_input_string()
      ));
    }, download_prompt_background.getGlobalBounds(), sf::Mouse::Button::Left);
  }
  else {
    can_search_string_scroll = false;
    data.search->draw_input_shadow();
  }

  data.search->draw();

  // Add Playlist

  window.draw(data.control_corner_shadow);
  window.draw(data.control_corner);
  window.draw(*data.add_playlist);

  new_click_event(click_events, "add_playlist", [](MenuData& menu_data) {
    popup_components.at("add_playlist")->hidden = false;
  }, data.add_playlist->getGlobalBounds(), sf::Mouse::Button::Left);


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
    pbar_progress_done.setFillColor(main_color);
    pbar_progress_done.setPosition(pbar_progress.getPosition());

    window.draw(pbar_background);
    window.draw(pbar_text);
    window.draw(pbar_doing_text);
    window.draw(pbar_progress);
    window.draw(pbar_progress_done);
  }

  // Add new playlist
  popup_components.at("add_playlist")->draw();


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
