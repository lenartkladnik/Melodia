#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <algorithm>
#include <math.h>
#include "include/player_menu.hpp"
#include "include/playlist_selector_menu.hpp"
#include "include/data.hpp"
#include "include/animation.hpp"
#include "include/events.hpp"

using namespace sf;

int main(int argc, char *argv[]) {
  // Load some assets

  sf::Image icon;
  if (!icon.loadFromFile(base_path_misc + "icon.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "icon.png'." << std::endl;
    return 1;
  }

  if (!default_font.openFromFile(base_path_misc + "base-font.ttf")) {
    std::cerr << "Error: Failed to load font 'base-font.tff'." << std::endl;
    return 1;
  }

  window.setIcon(icon.getSize(), icon.getPixelsPtr());

  MenuData menu_data;

  switch_to_playlist_selector(menu_data, window); // Start as the playlist selector


  getFontOffsetPixels(small_font_size);
  getFontOffsetPixels(medium_font_size);
  getFontOffsetPixels(medium_2_font_size);
  getFontOffsetPixels(large_font_size);


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

      if (const auto* resized = event->getIf<sf::Event::Resized>()) {
        // On resize:
        // - set new view
        // - unfocus search
        // - re-init

        // Ensure smooth resizing (https://en.sfml-dev.org/forums/index.php?topic=17747.0)
        window_size = {
          static_cast<float>(resized->size.x),
          static_cast<float>(resized->size.y)
        };
        default_view.setSize(window_size);
        default_view.setCenter({window_size.x / 2.f, window_size.y / 2.f});
        window.setView(default_view);

        switch (menu_data.type) {
          case (MenuData::Player): {
            if (!std::holds_alternative<MenuData::PlayerData>(menu_data.data)) {
              std::cerr << "Error: MenuData, should be of type Player" << std::endl;
              return 1;
            }

            auto& player = std::get<MenuData::PlayerData>(menu_data.data);

            player.data->search->unfocus();

            break;
          }

          case (MenuData::PlaylistSelector): {
            if (!std::holds_alternative<MenuData::PlaylistSelectorData>(menu_data.data)) {
              std::cerr << "Error: MenuData, should be of type PlaylistSelector" << std::endl;
              return 1;
            }

            auto& playlist_sel = std::get<MenuData::PlaylistSelector>(menu_data.data);

            playlist_sel.data->search->unfocus();

            // After the resize all items must be re-rendered
          switch_to_playlist_selector(menu_data, window);

          break;
        }
      }
    }

      if (!pause_main_input_handling) {
        on<sf::Event::MouseWheelScrolled>(*event, scroll_events,
          [&](const auto* e, const auto& item) { return item.can_scroll; },
          [&](const auto* e, const auto* item) { item->scroll_offset -= e->delta * scroll_speed; }
        );

        on<sf::Event::MouseButtonPressed>(*event, click_events,
          [&](const auto* e, const auto& item) { return item.mouse_button == e->button; },
          [&](const auto* e, const auto* item) { item->function(menu_data); }
        );

        on<sf::Event::MouseButtonPressed>(*event, focus_events,
          [&](const auto* e, const auto& item) { return item.mouse_button == e->button; },
          [&](const auto* e, const auto* item) {
            auto pos = window.mapPixelToCoords(e->position, item->view);
            item->function(menu_data, pos);
          }
        );

        on<sf::Event::TextEntered>(*event, text_events,
          [&](const auto* e, const auto& item) { return item.input_component->is_active() && !item.input_component->hidden; },
          [&](const auto* e, const auto* item) { item->input_component->write_input(e->unicode); }
        );

        // Update all text
        for (const auto& each : text_events) {
          if (each.input_component->is_active() && !each.input_component->hidden) {
            each.input_component->update();
          }
        }

        // Menu specific events
        switch (menu_data.type) {
          case (MenuData::Player): {
            if (!std::holds_alternative<MenuData::PlayerData>(menu_data.data)) {
              std::cerr << "Error: MenuData, should be of type Player" << std::endl;
              return 1;
            }

            auto& player = std::get<MenuData::PlayerData>(menu_data.data);

            // if (input_active) {
            //   if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            //     player.data->search->move_cursor_left();
            //   }
            //   else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            //     player.data->search->move_cursor_right();
            //   }
            // } else {
              // Keyboard controls (turned off when input_active)

            //   if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
            //     if (keyPressed->scancode == sf::Keyboard::Scancode::Space) {
            //       player.music->toggle_play_state();
            //     }
            //   }
            // }

            // if (const auto* text = event->getIf<Event::TextEntered>()) {
            //   if (input_active) {
            //     auto input = text->unicode;

            //     player.data->search->write_input(input);
            //   }
            // }

            // if (mouseButtonPressed) {
            //   auto pos = window.mapPixelToCoords(mouseButtonPressed->position);

            // TODO:  if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
            //          if (player.data->playlist_selector->getGlobalBounds().contains(pos) && !player.data->queue_expanded) {
            //            switch_to_playlist_selector(menu_data, window);
            //            break;
            //          }


                // ----------
                // All inputs for the queue are handled in the player_menu.cpp file where the queue is generated on the fly
                // ----------

                // Actions that require focusing an element

                // if (player.data->search->background_bounds().contains(pos)) { // Search is active
                //   player.data->search->focus(pos);
                // }
                // else { // Clicked anywhere else
                //   player.data->search->unfocus();
                // }
              // }
            // }

          break;
          }

          case (MenuData::PlaylistSelector): {
            if (!std::holds_alternative<MenuData::PlaylistSelectorData>(menu_data.data)) {
              std::cerr << "Error: MenuData, should be of type PlaylistSelector" << std::endl;
              return 1;
            }

            auto& playlist_sel = std::get<MenuData::PlaylistSelector>(menu_data.data);

            playlist_sel.reset_cursor = true;

            // Progress bar
            if (!progress_bar_string.empty()) {
              if (progress_bar_amount == progress_bar_total) {
                download_song_thread->join();
                pause_main_input_handling = false;
                playlist_sel.data->search->force_input_refresh(); // Reset the search (so the new downloaded song is shown)
                progress_bar_string = ""; // Reset so that the .empty check above succeeds
              }
            }

            // if (input_active) {
            //   if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            //     playlist_sel.data->search->move_cursor_left();
            //   }
            //   else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            //     playlist_sel.data->search->move_cursor_right();
            //  }
            // }

            // if (const auto* text = event->getIf<Event::TextEntered>()) {
            //   if (input_active) {
            //     auto input = text->unicode;

            //     playlist_sel.data->search->write_input(input);
            //   }
            // }

            const auto* mouseWheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>();
            if (mouseWheelScrolled) {
              if (playlist_sel.data->search->not_empty())
                search_res_click_events.clear();
            }

            const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>();
            if (mouseButtonPressed) {
              auto pos = window.mapPixelToCoords(mouseButtonPressed->position);

              if (playlist_sel.data->search->not_empty()) {
                for (const auto& each : search_res_click_events) {
                  if ((each.mouse_button == mouseButtonPressed->button) && each.bounds.contains(pos)) {
                    each.function(menu_data);
                  }
                }
              }

              // if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
                // Unfocusing requires detection of clicks on the whole
                // screen so I have not gotten around to implementing it in
                // a click event

              //   if (playlist_sel.data->search->background_bounds().contains(pos)) { // Search is active
              //     playlist_sel.data->search->focus(pos);
              //   }
              //   else { // Clicked anywhere else
              //     playlist_sel.data->search->unfocus();
              //   }
              // }
            }

          break;
          }
          default:
            std::cerr << "Error: Invalid menu selected." << std::endl;
            switch_to_playlist_selector(menu_data, window);

          break;
        }
      }
    } // end while for event handling

    switch (menu_data.type) {
      case (MenuData::Player): {
        if (!std::holds_alternative<MenuData::PlayerData>(menu_data.data)) {
            std::cerr << "Error: MenuData, should be of type Player" << std::endl;
            return 1;
        }

        auto& player = std::get<MenuData::PlayerData>(menu_data.data);

        // if (input_active) {
        //   player.data->search->update();
        // }

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
          player.data = init_player(window, player.song_path, player.song_id, player.playlist);
          player.data->cover.setTexture(player.data->cover_texture.get()); // Ensure the cover art texture is set

          // Reset state
          if (queue_was_open) instant_open_queue(&player); // Instant so the opening animation is not visible
          if (player.live_mode) player.data->live->setTexture(*player.data->live_full_tex);
        }

        auto pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        // Hover effects

        // TODO: Figure out how to only change hover state when the obj has hidden false
        if (player.data->search->background_bounds().contains(pos)) {
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
            player.data->search->cancel_input_bounds().contains(pos) ||
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

        if (player.data) display_player(player, window);
        else player.playing_song_id = -1; // Something went wrong re-init

      break;
      }

      case (MenuData::PlaylistSelector): {
        auto& playlist_sel = std::get<MenuData::PlaylistSelector>(menu_data.data);

        // if (input_active) {
        //   playlist_sel.data->search->update();
        // }

        auto pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        // Hover effects

        // TODO: Figure out how to only change hover state when the obj has hidden false
        if (playlist_sel.data->search->background_bounds().contains(pos) && !playlist_sel.data->search->cancel_input_bounds().contains(pos)) {
          window.setMouseCursor(text_cursor);
          playlist_sel.reset_cursor = false;
        }
        else if (
            playlist_sel.data->add_playlist->getGlobalBounds().contains(pos) ||
            playlist_sel.data->search->cancel_input_bounds().contains(pos) ||
            !playlist_sel.reset_cursor
          ) {

          window.setMouseCursor(hand_cursor);

          playlist_sel.reset_cursor = false;
        }
        else if (playlist_sel.reset_cursor) {
          window.setMouseCursor(default_cursor);
        }


        if (playlist_sel.data) {
          if (!display_playlist_selector(playlist_sel, window, menu_data)) break; // false returned when switched to new menu
        }

      break;
      }
    }
  }

  return 0;
}

