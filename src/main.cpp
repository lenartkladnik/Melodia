#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <algorithm>
#include <math.h>
#include "menus.hpp"
#include "data.hpp"
#include "animation.hpp"

using namespace sf;

sf::Clock sf_clock;

int main(int argc, char *argv[]) {
  // Load some assets

  sf::Image icon;
  if (!icon.loadFromFile("misc/icon.png")) {
    std::cerr << "Error: Failed to load 'misc/icon.png'." << std::endl;
  }

  window.setIcon(icon.getSize(), icon.getPixelsPtr());

  MenuData menu_data;

  auto open_queue = [](auto* player, auto speed){
    player->data->queue_toggle.setTexture(*player->data->side_contract_tex);
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
    player->data->queue_toggle.setTexture(*player->data->side_expand_tex);
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
    player->data->queue_toggle.setTexture(*player->data->side_contract_tex);
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

      switch (menu_data.type) {
        case (MenuData::Player): {
          if (!std::holds_alternative<MenuData::PlayerData>(menu_data.data)) {
            std::cerr << "Error: MenuData, should be of type Player" << std::endl;
            return 1;
          }

          auto& player = std::get<MenuData::PlayerData>(menu_data.data);

          if (player.dragging_queue == -1)
            player.reset_cursor = true; // Reset the value to true on each iteration

          // Keyboard controls (turned off when player.search_active)
          if (!player.search_active) {
            if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
              if (keyPressed->scancode == sf::Keyboard::Scancode::Space) {
                player.music.toggle_play_state();
              }
            }
          } else {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) && player.cursor_pos > 0) {
              // Keep cursor solid while changing cursor pos
              sf_clock.restart();
              player.show_cursor = true;

              player.cursor_pos--;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) && player.cursor_pos < player.data->search_string.size()) {
              // Keep cursor solid while changing cursor pos
              sf_clock.restart();
              player.show_cursor = true;

              player.cursor_pos++;
            }
          }

          if (const auto* text = event->getIf<Event::TextEntered>()) {
            if (player.search_active) {
              auto input = text->unicode;

              // Keep cursor solid while inputting
              sf_clock.restart();
              player.show_cursor = true;

              if (input < 32 || input > 126) {
                switch (input) {
                  case 8:
                    if (player.data->search_string.size() > 0) {
                      player.cursor_pos--;
                      player.data->search_string.erase(player.cursor_pos, 1);
                    }
                    break;
                }
              }
              else if (player.data->search_string.size() < queue_search_max_char) {
                player.data->search_string.insert(player.cursor_pos, char32_to_utf8(input));
                player.cursor_pos++;
              }
            }
          }

          if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
              auto pos = window.mapPixelToCoords(mouseButtonPressed->position);
              if (player.data->main_control.getGlobalBounds().contains(pos)) { // Clicked on the play / pause button
                player.music.toggle_play_state();
              }

              else if (player.data->next_control.getGlobalBounds().contains(pos)) { // Next song
                player.song_id = player.queue[0];
                done_playing(player.queue, player.past_queue);
              }

              else if (player.data->previous_control.getGlobalBounds().contains(pos)) { // Previous song
                auto old_id = player.song_id;
                if (player.past_queue.size() > 1) {
                  auto idx = player.past_queue.size() - 2;

                  player.song_id = player.past_queue[idx];
                  player.past_queue.erase(player.past_queue.begin() + idx);

                  player.queue.erase(std::find(player.queue.begin(), player.queue.end(), old_id));
                  player.queue.insert(player.queue.begin(), old_id);
                }
              }

              else if (player.data->trash.getGlobalBounds().contains(pos)) { // Delete song
                std::cout << "TODO: Delete song" << std::endl;
              }

              else if (player.data->manage_playlist.getGlobalBounds().contains(pos)) { // Manage playlist
                std::cout << "TODO: Manage playlist popup" << std::endl;
              }

              else if (player.data->favorite.getGlobalBounds().contains(pos)) { // Toggle favorite song
                std::cout << "TODO: Toggle favorite song" << std::endl;
                if (player.data->favorite_empty_tex->getNativeHandle() == player.data->favorite.getTexture().getNativeHandle()) {
                  // Favorite
                  player.data->favorite.setTexture(*player.data->favorite_full_tex);
                }
                else {
                  // Un-favorite
                  player.data->favorite.setTexture(*player.data->favorite_empty_tex);
                }
              }

              else if (player.data->edit.getGlobalBounds().contains(pos)) { // Edit song
                std::cout << "TODO: Edit song" << std::endl;
              }

              else if (player.data->progress.getGlobalBounds().contains(pos)) { // Clicked on the play progress bar -> start seeking
                player.seeking = true;
                player.music.silent_mute();
                player.was_playing = player.music.is_playing();
              }

              else if (player.data->queue_toggle.getGlobalBounds().contains(pos)) { // Toggle the sidebar with the queue
                player.data->queue_expanded = !player.data->queue_expanded;

                if (player.data->queue_expanded) open_queue(&player, move_speed);
                else close_queue(&player, move_speed);
              }

              else if (player.data->vol_icon.getGlobalBounds().contains(pos)) { // Clicked on the audio icon -> toggle mute
                if (player.data->volume_tex->getNativeHandle() == player.data->vol_icon.getTexture().getNativeHandle()) {
                  // Mute
                  player.data->vol_icon.setTexture(*player.data->mute_tex);

                  player.music.mute();
                }
                else {
                  // Un-mute
                  player.data->vol_icon.setTexture(*player.data->volume_tex);

                  player.music.unmute();
                }
              }

              else if (player.data->vol_slider.getGlobalBounds().contains(pos)) { // Clicked on the volume slider -> start change volume
                player.volume_slider_active = true;
              }

              else if (player.data->live.getGlobalBounds().contains(pos)) { // Clicked on the live toggle
                std::cout << "TODO: Toggle live mode" << std::endl;
                if (player.data->live_full_tex->getNativeHandle() == player.data->live.getTexture().getNativeHandle()) {
                  player.data->live.setTexture(*player.data->live_empty_tex);
                  player.live_mode = false;
                }
                else {
                  player.data->live.setTexture(*player.data->live_full_tex);
                  player.live_mode = true;
                }
              }

              else if (player.data->cancel_queue_search.getGlobalBounds().contains(pos) && !player.data->search_string.empty()) { // Clicked the cancel_queue_search button when the search was active -> clear search
                player.data->search_string = "";
                player.cursor_pos = 0;
              }

              else if (player.data->playlist_selector.getGlobalBounds().contains(pos) && !player.data->queue_expanded) { // Clicked on the playlist selector
                std::cout << "TODO: Playlist selector" << std::endl;
              }

              // ----------
              // All inputs for the queue are handled in the menus.cpp file where the queue is generated on the fly
              // ----------

              // Actions that require focusing an element

              if (player.data->search_background.getGlobalBounds().contains(pos)) { // Search is active
                if (player.search_active) { // Set cursor pos
                  if (player.data->search_string.empty()) {
                    player.cursor_pos = 0;
                  }
                  else {
                    auto c_pos = round((pos.x - player.data->search_before_cursor.getPosition().x) / (player.data->search_before_cursor.getGlobalBounds().size.x + player.data->search_after_cursor.getGlobalBounds().size.x) * player.data->search_string.size());

                    // Keep cursor solid while changing cursor pos
                    sf_clock.restart();
                    player.show_cursor = true;

                    if (c_pos > player.data->search_string.size()) {
                      player.cursor_pos = player.data->search_string.size();
                    }
                    else {
                      player.cursor_pos = c_pos;
                    }
                  }
                }
                else {
                  player.search_active = true;
                  player.data->search_before_cursor.setFillColor(text_color);
                  player.data->search_before_cursor.setString(player.data->search_string.substr(0, player.cursor_pos));
                  player.data->search_after_cursor.setString(player.data->search_string.substr(player.cursor_pos));
                }
              }
              else { // Clicked anywhere else
                player.search_active = false;
                player.show_cursor = false;

                if (player.data->search_string.size() == 0) {
                  player.data->search_before_cursor.setFillColor(light_text_color);
                  player.data->search_before_cursor.setString("Search");
                }
              }
            }
          }
        break;
        }
        default:
          std::cerr << "Error: Invalid menu selected." << std::endl;
          menu_data.type = MenuData::Main;
        break;
      }
    }

    switch (menu_data.type) {
      case (MenuData::Player): {
        if (!std::holds_alternative<MenuData::PlayerData>(menu_data.data)) {
            std::cerr << "Error: MenuData, should be of type Player" << std::endl;
            return 1;
        }

        auto& player = std::get<MenuData::PlayerData>(menu_data.data);

        if (player.search_active) {
          if (sf_clock.getElapsedTime() >= sf::milliseconds(500)) {
            sf_clock.restart();
            player.show_cursor = !player.show_cursor;
          }

          player.data->search_before_cursor.setString(player.data->search_string.substr(0, player.cursor_pos));
          player.data->search_after_cursor.setString(player.data->search_string.substr(player.cursor_pos));
        }

        if (player.seeking) {
          if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            player.seeking = false;
            player.music.unmute();
            if (player.was_playing) player.music.play();
          }

          auto coords_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
          float progress_pos = (coords_pos.x - player.data->progress.getPosition().x) / player.data->progress.getGlobalBounds().size.x;
          progress_pos = std::clamp(progress_pos, 0.f, 1.f);
          player.music.seek(progress_pos);
        }
        else if (player.music.is_stopped()) { // The current song has done playing (and not seeking)
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
          player.music.set_volume(vol_pos);
        }

        // New song has been set to play

        if (player.playing_song_id != player.song_id) {
          bool queue_was_open = false;

          if (player.data.has_value()) {
            queue_was_open = player.data->queue_expanded;
          }

          player.song_path = construct_song_path(player.song_id);

          if (!player.music.load(player.song_path + ".ogg")) {
            continue;
          }

          player.playing_song_id = player.song_id;

          player.music.play();
          player.data = init_player(window, player.song_path, player.playlist);
          player.data->cover.setTexture(&player.data->cover_texture); // Ensure the cover art texture is set

          // Reset state
          if (queue_was_open) instant_open_queue(&player); // Instant so the opening animation is not visible
          if (player.live_mode) player.data->live.setTexture(*player.data->live_full_tex);
        }

        auto pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        // Hover effects

        if (player.data->search_background.getGlobalBounds().contains(pos)) {
          window.setMouseCursor(text_cursor);
          player.reset_cursor = false;
        }
        else if (
            player.data->main_control.getGlobalBounds().contains(pos) ||
            player.data->next_control.getGlobalBounds().contains(pos) ||
            player.data->previous_control.getGlobalBounds().contains(pos) ||
            player.data->trash.getGlobalBounds().contains(pos) ||
            player.data->manage_playlist.getGlobalBounds().contains(pos) ||
            player.data->favorite.getGlobalBounds().contains(pos) ||
            player.data->edit.getGlobalBounds().contains(pos) ||
            player.data->progress.getGlobalBounds().contains(pos) ||
            player.data->queue_toggle.getGlobalBounds().contains(pos) ||
            player.data->vol_icon.getGlobalBounds().contains(pos) ||
            player.data->vol_slider.getGlobalBounds().contains(pos) ||
            player.data->live.getGlobalBounds().contains(pos) ||
            player.data->cancel_queue_search.getGlobalBounds().contains(pos) ||
            !player.reset_cursor
          ) {

          window.setMouseCursor(hand_cursor);

          player.reset_cursor = false;
        }
        else if (player.reset_cursor) {
          window.setMouseCursor(default_cursor);
        }

        if (player.data) display_player(player, window);
        else player.playing_song_id = -1; // Something went wrong re-init
      break;
      }
    }
  }

  return 0;
}

