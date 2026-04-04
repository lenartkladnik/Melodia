#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <algorithm>
#include <math.h>
#include "include/menus.hpp"
#include "include/player_menu.hpp"
#include "include/playlist_selector_menu.hpp"
#include "include/data.hpp"
#include "include/animation.hpp"

using namespace sf;

int main(int argc, char *argv[]) {
  // Load some assets

  sf::Image icon;
  if (!icon.loadFromFile(base_path_misc + "icon.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "icon.png'." << std::endl;
    return 1;
  }

  sf::Font default_font;
  if (!default_font.openFromFile(base_path_misc + "JetBrainsMono-Regular.ttf")) {
    std::cerr << "Error: Failed to load font 'JetBrainsMono-Regular.tff'." << std::endl;
    return 1;
  }

  window.setIcon(icon.getSize(), icon.getPixelsPtr());

  // if (!download_song_from_query("Clint Eastwood")) {
  //   std::cout << "Failed to download song" << std::endl;
  // }

  MenuData menu_data;

  switch_to_playlist_selector(menu_data, window, default_font); // Start as the playlist selector

  auto open_queue = [](auto* player, auto speed){
    player->data->queue_toggle->setTexture(*player->data->side_contract_tex);
    player->data->queue_expanded = true;

    animate_move_all_x(
      {
        &player->data->queue_background,
        &player->data->queue_background_shadow
      },
      -10.f,
      speed,
      &player->data->queue_half_expanded,
      true,
      AnimationStage::half
    );
  };

  auto close_queue = [](auto* player, auto speed) {
    player->data->queue_toggle->setTexture(*player->data->side_expand_tex);
    player->data->queue_half_expanded = false;
    player->data->queue_expanded = false;

    animate_move_x(
      player->data->queue_background,
      queue_contracted_width - player->data->queue_background.getGlobalBounds().size.x,
        -speed
      );
    animate_move_x(
      player->data->queue_background_shadow,
      queue_contracted_width - player->data->queue_background.getGlobalBounds().size.x + shadow_offset,
        -speed
    );
  };

  auto instant_open_queue = [](auto* player){
    player->data->queue_toggle->setTexture(*player->data->side_contract_tex);
    player->data->queue_expanded = true;
    player->data->queue_half_expanded = true;

    float diff = player->data->queue_background_shadow.getPosition().x - player->data->queue_background.getPosition().x;

    player->data->queue_background.setPosition({-10.f, player->data->queue_background.getPosition().y});
    player->data->queue_background_shadow.setPosition({-10.f + diff, player->data->queue_background_shadow.getPosition().y});
  };

  while (window.isOpen()) {
    while (const std::optional event = window.pollEvent()) {
      if (event->is<sf::Event::Closed>())
        window.close();

      if (!pause_main_input_handling) {
        switch (menu_data.type) {
          case (MenuData::Player): {
            if (!std::holds_alternative<MenuData::PlayerData>(menu_data.data)) {
              std::cerr << "Error: MenuData, should be of type Player" << std::endl;
              return 1;
            }

            auto& player = std::get<MenuData::PlayerData>(menu_data.data);

            if (search_active) {
              if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
                search_move_cursor_left();
              }
              else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
                search_move_cursor_right();
              }
            } else {
              // Keyboard controls (turned off when search_active)

              if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Space) {
                  player.music->toggle_play_state();
                }
              }
            }

            if (const auto* text = event->getIf<Event::TextEntered>()) {
              if (search_active) {
                auto input = text->unicode;

                search_input(input);
              }
            }

            if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
              auto pos = window.mapPixelToCoords(mouseButtonPressed->position);

              for (const auto& each : click_events) {
                if ((each.mouse_button == mouseButtonPressed->button) && each.bounds.contains(pos)) {
                  each.function(menu_data);
                }
              }

              if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
                if (player.data->playlist_selector->getGlobalBounds().contains(pos) && !player.data->queue_expanded) {
                  switch_to_playlist_selector(menu_data, window, default_font);
                  break;
                }


                // ----------
                // All inputs for the queue are handled in the player_menu.cpp file where the queue is generated on the fly
                // ----------

                // Actions that require focusing an element

                if (player.data->search_background.getGlobalBounds().contains(pos)) { // Search is active
                  search_focus(pos, *player.data->search_before_cursor, *player.data->search_after_cursor);
                }
                else { // Clicked anywhere else
                  search_unfocus(*player.data->search_before_cursor, *player.data->search_after_cursor);
                }
              }
            }

          break;
          }

          case (MenuData::PlaylistSelector): {
            if (!std::holds_alternative<MenuData::PlaylistSelectorData>(menu_data.data)) {
              std::cerr << "Error: MenuData, should be of type PlaylistSelector" << std::endl;
              return 1;
            }

            auto& playlist_sel = std::get<MenuData::PlaylistSelector>(menu_data.data);

            playlist_sel.reset_cursor = true;

            if (search_active) {
              if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
                search_move_cursor_left();
              }
              else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
                search_move_cursor_right();
              }
            }

            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
              playlist_sel.data = init_playlist_selector(window, default_font); // Recalculate for the new size
            }

            if (const auto* text = event->getIf<Event::TextEntered>()) {
              if (search_active) {
                auto input = text->unicode;

                search_input(input);
              }
            }

            if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
              auto pos = window.mapPixelToCoords(mouseButtonPressed->position);

              for (const auto& each : click_events) {
                if ((each.mouse_button == mouseButtonPressed->button) && each.bounds.contains(pos)) {
                  each.function(menu_data);
                }
              }

              if (!search_string.empty()) {
                for (const auto& each : search_res_click_events) {
                  if ((each.mouse_button == mouseButtonPressed->button) && each.bounds.contains(pos)) {
                    each.function(menu_data);
                  }
                }
              }

              if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
                if (playlist_sel.data->add_playlist.getGlobalBounds().contains(pos)) {
                  std::cout << "TODO: Clicked on add playlist button" << std::endl;
                }

                // Actions that require focusing an element

                if (playlist_sel.data->search_background.getGlobalBounds().contains(pos)) { // Search is active
                  search_focus(pos, playlist_sel.data->search_before_cursor, playlist_sel.data->search_after_cursor);
                }
                else { // Clicked anywhere else
                  search_unfocus(playlist_sel.data->search_before_cursor, playlist_sel.data->search_after_cursor);
                }
              }
            }

          break;
          }
          default:
            std::cerr << "Error: Invalid menu selected." << std::endl;
            switch_to_playlist_selector(menu_data, window, default_font);

          break;
        }
      } // end while for event polling
    } // end if !pause_main_input_handling

    // Progress bar
    if (!progress_bar_string.empty()) {
      std::cout << "Progress bar: " << progress_bar_string << " " << progress_bar_amount / progress_bar_total * 100 << "%" << std::endl;
      if (progress_bar_amount == progress_bar_total) {
        download_song_thread->join();
        pause_main_input_handling = false;
        prev_search_string = ""; // Reset the search (so the new downloaded song is shown)
        progress_bar_string = ""; // Reset so that the .empty check above succeeds
      }
    }

    switch (menu_data.type) {
      case (MenuData::Player): {
        if (!std::holds_alternative<MenuData::PlayerData>(menu_data.data)) {
            std::cerr << "Error: MenuData, should be of type Player" << std::endl;
            return 1;
        }

        auto& player = std::get<MenuData::PlayerData>(menu_data.data);

        if (search_active) {
          search_running(*player.data->search_before_cursor, *player.data->search_after_cursor);
        }

        if (player.seeking) {
          if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            player.seeking = false;
            player.music->unmute();
            if (player.was_playing) player.music->play();
          }

          auto coords_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
          float progress_pos = (coords_pos.x - player.data->progress.getPosition().x) / player.data->progress.getGlobalBounds().size.x;
          progress_pos = std::clamp(progress_pos, 0.f, 1.f);
          player.music->seek(progress_pos);
        }
        else if (player.music->is_stopped()) { // The current song has done playing (and not seeking)
          player.song_id = player.queue[0];
          done_playing(player.queue, player.past_queue);
        }

        if (player.volume_slider_active) {
          if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            player.volume_slider_active = false;
          }

          auto coords_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
          float vol_pos = (coords_pos.x - player.data->vol_slider.getPosition().x) / player.data->vol_slider.getGlobalBounds().size.x;
          vol_pos = std::clamp(vol_pos, 0.f, 1.f);
          player.music->set_volume(vol_pos);
        }

        // New song has been set to play

        if (player.playing_song_id != player.song_id) {
          bool queue_was_open = false;

          if (player.data != nullptr) {
            queue_was_open = player.data->queue_expanded;
          }

          player.song_path = construct_song_path(player.song_id);

          if (!player.music->load(player.song_path + ".mp3")) {
            continue;
          }

          player.playing_song_id = player.song_id;

          player.music->play();
          player.data = init_player(window, player.song_path, player.song_id, player.playlist, default_font);
          player.data->cover.setTexture(player.data->cover_texture.get()); // Ensure the cover art texture is set

          // Reset state
          if (queue_was_open) instant_open_queue(&player); // Instant so the opening animation is not visible
          if (player.live_mode) player.data->live->setTexture(*player.data->live_full_tex);
        }

        auto pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        // Hover effects

        if (player.data->search_background.getGlobalBounds().contains(pos)) {
          window.setMouseCursor(text_cursor);
          player.reset_cursor = false;
        }
        else if (
            player.data->main_control->getGlobalBounds().contains(pos) ||
            player.data->next_control->getGlobalBounds().contains(pos) ||
            player.data->previous_control->getGlobalBounds().contains(pos) ||
            player.data->trash->getGlobalBounds().contains(pos) ||
            player.data->manage_playlist->getGlobalBounds().contains(pos) ||
            player.data->favorite->getGlobalBounds().contains(pos) ||
            player.data->edit->getGlobalBounds().contains(pos) ||
            player.data->progress.getGlobalBounds().contains(pos) ||
            player.data->queue_toggle->getGlobalBounds().contains(pos) ||
            player.data->vol_icon->getGlobalBounds().contains(pos) ||
            player.data->vol_slider.getGlobalBounds().contains(pos) ||
            player.data->live->getGlobalBounds().contains(pos) ||
            player.data->cancel_queue_search->getGlobalBounds().contains(pos) ||
            (player.data->playlist_selector->getGlobalBounds().contains(pos) && !player.data->queue_expanded) ||
            !player.reset_cursor
          ) {

          window.setMouseCursor(hand_cursor);
        }
        else if (player.reset_cursor) {
          window.setMouseCursor(default_cursor);
        }
        else {
          player.reset_cursor = true;
        }

        if (player.data) display_player(player, window, default_font);
        else player.playing_song_id = -1; // Something went wrong re-init

      break;
      }

      case (MenuData::PlaylistSelector): {
        auto& playlist_sel = std::get<MenuData::PlaylistSelector>(menu_data.data);

        if (search_active) {
          search_running(playlist_sel.data->search_before_cursor, playlist_sel.data->search_after_cursor);
        }

        auto pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        // Hover effects

        if (playlist_sel.data->search_background.getGlobalBounds().contains(pos) && !playlist_sel.data->cancel_search.getGlobalBounds().contains(pos)) {
          window.setMouseCursor(text_cursor);
          playlist_sel.reset_cursor = false;
        }
        else if (
            playlist_sel.data->add_playlist.getGlobalBounds().contains(pos) ||
            playlist_sel.data->cancel_search.getGlobalBounds().contains(pos) ||
            !playlist_sel.reset_cursor
          ) {

          window.setMouseCursor(hand_cursor);

          playlist_sel.reset_cursor = false;
        }
        else if (playlist_sel.reset_cursor) {
          window.setMouseCursor(default_cursor);
        }

        if (playlist_sel.data) {
          if (!display_playlist_selector(playlist_sel, window, menu_data, default_font)) break; // false returned when switched to new menu
        }

      break;
      }
    }
  }

  return 0;
}

