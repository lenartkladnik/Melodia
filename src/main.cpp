#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <algorithm>
#include <math.h>
#include <clocale>

#include "include/components.hpp"
#include "include/player_menu.hpp"
#include "include/playlist_selector_menu.hpp"
#include "include/data.hpp"
#include "include/animation.hpp"
#include "include/events.hpp"
#include "include/storage_handler.hpp"
#include "include/signals.hpp"
#include "include/scheduler.hpp"
#include "include/playlist_scraper.hpp"
#include "include/download.hpp"

using namespace sf;

int app() {
  if (!ensure_storage())
    return 1;

  set_colors();

  // Load some assets

  sf::Image icon;
  if (!icon.loadFromFile(base_path_misc + "icon.png")) {
    throw std::runtime_error("Failed to load '" + base_path_misc + "icon.png'.");
  }

  if (!default_font.openFromFile(base_path_misc + "base-font.ttf")) {
    throw std::runtime_error("Failed to load font '" + base_path_misc + "base-font.tff'.");
  }
  default_font.setSmooth(true);


  sf::RenderWindow window_popup_loading(sf::VideoMode({300, 100}), "Melodia - Loading", sf::Style::None);
  sf::Texture icon_tex;
  if (!icon_tex.loadFromImage(icon)) {
    throw std::runtime_error("Failed to load icon from image.\n");
  }
  sf::Sprite icon_sprite(icon_tex);
  icon_sprite.setPosition({
    window_popup_loading.getSize().x / 2 - icon_sprite.getGlobalBounds().size.x / 2,
    window_popup_loading.getSize().y - icon_sprite.getGlobalBounds().size.y - 2.f
  });
  sf::Text loading_text(default_font, "Loading...");
  loading_text.setCharacterSize(24);
  loading_text.setFillColor(text_color);
  loading_text.setStyle(sf::Text::Bold);
  loading_text.setPosition({
    window_popup_loading.getSize().x / 2 - loading_text.getGlobalBounds().size.x / 2,
    window_popup_loading.getSize().y / 2 - loading_text.getGlobalBounds().size.y / 2 - 10.f
  });
  window_popup_loading.clear(sf::Color(background_color));
  window_popup_loading.draw(loading_text);
  window_popup_loading.draw(icon_sprite);
  window_popup_loading.display();

  // Things that are done during loading

  yt_dlp_path = find_yt_dlp(); // find yt-dlp on PATH and download if isn't found
  rasterize_textures(); // svg (./misc) -> png (./misc/rasters)

  window_popup_loading.close();

  if (!window.resize(window_base_size)) {
    throw std::runtime_error("Failed to resize window render texture.");
  }

  set_window(sf::State::Windowed);
  std::setlocale(LC_ALL, "en_US.UTF-8");

  render_window.setIcon(icon.getSize(), icon.getPixelsPtr());

  switch_to_playlist_selector(); // Start as the playlist selector
  // switch_to_player(window, render_window, "tmp");

  getFontOffsetPixels(small_font_size);
  getFontOffsetPixels(medium_font_size);
  getFontOffsetPixels(medium_2_font_size);
  getFontOffsetPixels(large_font_size);


  // Queue animations (unused)
  //
  // auto open_queue = [](auto* player, auto speed){
  //   player->data->queue_toggle->setTexture(*player->data->side_contract_tex);
  //   player->data->queue_expanded = true;
  //
  //   animate_move_all_x(
  //     {
  //       &player->data->queue_background,
  //       &player->data->queue_background_shadow
  //     },
  //     -10.f,
  //     speed,
  //     &player->data->queue_half_expanded,
  //     true,
  //     AnimationStage::half
  //   );
  // };
  //
  // auto close_queue = [](auto* player, auto speed) {
  //   player->data->queue_toggle->setTexture(*player->data->side_expand_tex);
  //   player->data->queue_half_expanded = false;
  //   player->data->queue_expanded = false;
  //
  //   animate_move_x(
  //     player->data->queue_background,
  //     queue_contracted_width - player->data->queue_background.getGlobalBounds().size.x,
  //       -speed
  //     );
  //   animate_move_x(
  //     player->data->queue_background_shadow,
  //     queue_contracted_width - player->data->queue_background.getGlobalBounds().size.x + shadow_offset,
  //       -speed
  //   );
  // };

  auto instant_open_queue = [](auto* player){
    player->data->queue_toggle->setTexture(*player->data->side_contract_tex);
    player->data->queue_expanded = true;
    player->data->queue_half_expanded = true;

    float diff = player->data->queue_background_shadow.getPosition().x - player->data->queue_background.getPosition().x;

    player->data->queue_background.setPosition({-10.f, player->data->queue_background.getPosition().y});
    player->data->queue_background_shadow.setPosition({-10.f + diff, player->data->queue_background_shadow.getPosition().y});
  };

  while (render_window.isOpen()) {
    for (auto& task : tasks) {
      task.tick();
    }
    remove_done_tasks();

    while (const std::optional event = render_window.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        render_window.close();
        cleanup();
      } else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
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
        render_window.setView(default_view);
        window.setView(default_view);
        if (!window.resize(resized->size)) {
          throw std::runtime_error("Failed to resize window render texture.");
        }

        switch (menu_data.type) {
          case (MenuData::Player): {
            if (!std::holds_alternative<MenuData::PlayerData>(menu_data.data)) {
              throw std::runtime_error("MenuData should be of type MenuData::Player");
            }

            auto& player = std::get<MenuData::PlayerData>(menu_data.data);

            player.data->search->unfocus();

            break;
          }

          case (MenuData::PlaylistSelector): {
            if (!std::holds_alternative<MenuData::PlaylistSelectorData>(menu_data.data)) {
              throw std::runtime_error("MenuData should be of type MenuData::PlaylistSelector");
            }

            auto& playlist_sel = std::get<MenuData::PlaylistSelector>(menu_data.data);

            playlist_sel.data->search->unfocus();

            // After the resize all items must be re-rendered
            switch_to_playlist_selector();

            break;
          }
        }
      } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
        switch (keyPressed->code) {
          case sf::Keyboard::Key::F11:
            if (is_fullscreen)
              set_window(sf::State::Windowed);
            else
              set_window(sf::State::Fullscreen);
            break;

          case sf::Keyboard::Key::Space:
            play_toggle_signal.emit();
            break;

          case sf::Keyboard::Key::Enter:
            confirm_signal.emit();
            break;

          case sf::Keyboard::Key::Left:
            left_signal.emit();
            break;

          case sf::Keyboard::Key::Right:
            right_signal.emit();
            break;

          case sf::Keyboard::Key::Escape:
            escape_signal.emit();
            break;

          default:
            break;
        }

        if (keyPressed->control) {
          switch (keyPressed->code) {
            case sf::Keyboard::Key::C:
              copy_signal.emit();
              break;

            case sf::Keyboard::Key::V:
              paste_signal.emit();
              break;

            case sf::Keyboard::Key::A:
              select_all_signal.emit();
              break;

            default:
              break;
          }
        }
      }

      if (!pause_main_input_handling) {
        on<sf::Event::MouseWheelScrolled>(*event, scroll_events,
          [&](const auto*, auto& item) { return *item.can_scroll; },
          [&](const auto* e, auto* item) { *item->scroll_offset -= e->delta * scroll_speed; },
          [](const auto*, auto*){}
        );

        on<sf::Event::MouseButtonPressed>(*event, click_events,
          [&](const auto* e, auto& item) { return item.mouse_button == e->button; },
          [&](const auto*, auto* item) {
            item->function();
          },
          [&](const auto*, auto* item) {
            if (item->component)
              item->component->unfocus();
          }
        );

        on_anywhere<sf::Event::MouseButtonReleased>(*event, release_events,
          [&](const auto* e, auto& item) { return item.mouse_button == e->button; },
          [&](const auto*, auto* item) {
            item->function();
          }
        );


        on<sf::Event::MouseButtonPressed>(*event, focus_events,
          [&](const auto* e, auto& item) { return item.mouse_button == e->button; },
          [&](const auto* e, auto* item) {
            auto pos = render_window.mapPixelToCoords(e->position, item->view);
            item->function(pos);
          },
          [&](const auto*, auto* item){
            item->else_function();
          }
        );

        on_anywhere<sf::Event::TextEntered>(*event, text_events,
          [&](const auto*, auto& item) { return item.input_component->is_active() && !item.input_component->is_hidden() && item.input_component->is_focused(); },
          [&](const auto* e, auto* item) { item->input_component->write_char(e->unicode); }
        );
      }

      // Menu specific events
      switch (menu_data.type) {
        case (MenuData::Player): {
          if (!std::holds_alternative<MenuData::PlayerData>(menu_data.data)) {
            throw std::runtime_error("MenuData should be of type MenuData::Player");
          }

          // auto& player = std::get<MenuData::PlayerData>(menu_data.data);

        break;
        }

        case (MenuData::PlaylistSelector): {
          if (!std::holds_alternative<MenuData::PlaylistSelectorData>(menu_data.data)) {
            throw std::runtime_error("MenuData should be of type MenuData::PlaylistSelector");
          }

          auto& playlist_sel = std::get<MenuData::PlaylistSelector>(menu_data.data);

          playlist_sel.reset_cursor = true;

          if (playlist_sel.data->search->is_focused()) {
            on<sf::Event::MouseButtonPressed>(*event, search_res_click_events,
              [&](const auto* e, const auto& item) { return item.mouse_button == e->button; },
              [&](const auto*, const auto* item) {
                item->function();
              },
              [&](const auto*, const auto* item) {
                if (item->component)
                  item->component->unfocus();
              }
            );

            on_anywhere<sf::Event::MouseButtonReleased>(*event, search_res_release_events,
              [&](const auto* e, const auto& item) { return item.mouse_button == e->button; },
              [&](const auto*, const auto* item) {
                item->function();
              }
            );
          }

          const auto* mouseWheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>();
          if (mouseWheelScrolled) {
            if (playlist_sel.data->search->is_focused()) {
              search_res_click_events.clear();
            }
          }

        break;
        }

        default: {
          throw std::runtime_error("Invalid menu selected");

        break;
        }
      }
    } // end while for event processing

    // Update all input components so they can blink their cursors and display any new text
    for (const auto& each : text_events) {
      if (each.input_component->is_active() && !each.input_component->is_hidden()) {
        each.input_component->update();
      }
    }

    // Hover effects TODO: Not working
    if (!pause_main_input_handling) {
      for (const auto& item : hover_events) {
        if (item.bounds.contains((sf::Vector2f)sf::Mouse::getPosition(render_window))) {
          if (!item.component->is_hidden()) {
            std::cout << "Hover on: " << item.id << "\n";
            item.component->on_hover();
          }
        }
        else {
          item.component->off_hover();
        }
      }
    }

    switch (menu_data.type) {
      case (MenuData::Player): {
        if (!std::holds_alternative<MenuData::PlayerData>(menu_data.data)) {
          throw std::runtime_error("MenuData should be of type MenuData::Player");
        }

        auto& player = std::get<MenuData::PlayerData>(menu_data.data);

        if (player.seeking) {
          if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            player.seeking = false;
            if (!player.music->was_muted)
              player.music->unmute();
            if (player.was_playing)
              player.music->play();
          }

          auto coords_pos = get_mouse_pos(render_window);
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

          auto coords_pos = get_mouse_pos(render_window);
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
          player.data = init_player(player.song_path, player.song_id, player.playlist);
          player.data->cover.setTexture(player.data->cover_texture.get()); // Ensure the cover art texture is set

          // Reset state
          if (queue_was_open) instant_open_queue(&player); // Instant so the opening animation is not visible
          if (player.live_mode) player.data->live->setTexture(*player.data->live_full_tex);
        }

        auto pos = get_mouse_pos(render_window);

        // Hover effects

        // TODO: Figure out how to only change hover state when the obj has hidden false
        if (player.data->search->background_bounds().contains(pos)) {
          render_window.setMouseCursor(text_cursor);
          player.reset_cursor = false;
        }
        else if (
            player.data->main_control->getGlobalBounds().contains(pos) ||
            player.data->next_control->getGlobalBounds().contains(pos) ||
            player.data->previous_control->getGlobalBounds().contains(pos) ||
            player.data->favorite->getGlobalBounds().contains(pos) ||
            player.data->progress.getGlobalBounds().contains(pos) ||
            player.data->queue_toggle->getGlobalBounds().contains(pos) ||
            player.data->vol_icon->getGlobalBounds().contains(pos) ||
            player.data->vol_slider.getGlobalBounds().contains(pos) ||
            player.data->live->getGlobalBounds().contains(pos) ||
            player.data->search->action_button_bounds().contains(pos) ||
            (player.data->playlist_selector->getGlobalBounds().contains(pos) && !player.data->queue_expanded) ||
            !player.reset_cursor
          ) {

          render_window.setMouseCursor(hand_cursor);
        }
        else if (player.reset_cursor) {
          render_window.setMouseCursor(default_cursor);
        }
        else {
          player.reset_cursor = true;
        }

        if (player.data) display_player();
        else player.playing_song_id = -1; // Something went wrong re-init

      break;
      }

      case (MenuData::PlaylistSelector): {
        auto& playlist_sel = std::get<MenuData::PlaylistSelector>(menu_data.data);

        auto pos = get_mouse_pos(render_window);

        // Hover effects

        // TODO: Figure out how to only change hover state when the obj has hidden false
        if (playlist_sel.data->search->background_bounds().contains(pos) && !playlist_sel.data->search->action_button_bounds().contains(pos)) {
          render_window.setMouseCursor(text_cursor);
          playlist_sel.reset_cursor = false;
        }
        else if (
            playlist_sel.data->search->action_button_bounds().contains(pos) ||
            !playlist_sel.reset_cursor
          ) {

          render_window.setMouseCursor(hand_cursor);

          playlist_sel.reset_cursor = false;
        }
        else if (playlist_sel.reset_cursor) {
          render_window.setMouseCursor(default_cursor);
        }


        if (playlist_sel.data) {
          if (!display_playlist_selector()) break; // false returned when switched to new menu
        }

      break;
      }
    }
  }

  return 0;
}

int main() {
  try {
    return app();
  } catch (...) {}

  // An exception was thrown, try to cleanup
  cleanup();
  return 1;
}

