#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "RoundedRectangleShape.hpp"
#include "data.hpp"
#include "menus.hpp"

std::unique_ptr<StaticPlayerData> init_player(sf::RenderWindow& window, const std::string& song_path, const std::string& playlist) {
  auto half = (float)(window_size.x / 2);
  auto third = (float)(window_size.x / 3);

  auto cover_size = third - offset;

  // Big cover art above the player controls

  sf::RoundedRectangleShape cover({cover_size, cover_size}, 8, main_n);

  auto cover_texture = std::make_shared<sf::Texture>();
  if (!cover_texture->loadFromFile(song_path + ".png")) {
    std::cerr << "Error: Failed to load '" << song_path << ".png" << "'." << std::endl;
  }
  cover_texture->setSmooth(true);

  cover.setTexture(cover_texture.get());
  cover.setPosition({half - cover.getGlobalBounds().size.x / 2, padding_top});

  sf::RoundedRectangleShape cover_shadow({cover_size, cover_size}, in_round, main_n);
  cover_shadow.setFillColor(background_shadow_color);
  cover_shadow.setPosition({cover.getPosition().x + shadow_offset, cover.getPosition().y + shadow_offset});


  // Artist and title information (for bellow the cover art)

  std::ifstream artist_file(song_path + ".artist");
  std::string artist_string = "";
  if (artist_file.good()) {
    std::getline(artist_file, artist_string);
  } else {
    std::cerr << "Error: Failed to read artist name from '" << song_path << ".artist" << "'.";
  }
  std::ifstream title_file(song_path + ".title");
  std::string title_string = "";
  if (title_file.good()) {
    std::getline(title_file, title_string);
  } else {
    std::cerr << "Error: Failed to read title from '" << song_path << ".title" << "'.";
  }

  sf::Text artist(default_font, artist_string);
  artist.setCharacterSize(18);
  artist.setPosition({half - artist.getGlobalBounds().size.x / 2, padding_top + cover_size + 50.f});
  artist.setFillColor(artist_color);
  sf::Text title(default_font, title_string);
  title.setCharacterSize(22);
  title.setPosition({half - title.getGlobalBounds().size.x / 2, padding_top + cover_size + 25.f});
  title.setFillColor(title_color);


  // Player controls

  auto play_tex = std::make_shared<sf::Texture>();
  if (!play_tex->loadFromFile(base_path_misc + "play.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "play.png'." << std::endl;
  }
  play_tex->setSmooth(true);

  auto pause_tex = std::make_shared<sf::Texture>();
  if (!pause_tex->loadFromFile(base_path_misc + "pause.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "pause.png'." << std::endl;
  }
  pause_tex->setSmooth(true);

  // Play / pause button
  sf::Sprite main_control(*play_tex);
  main_control.setPosition({(float)(half - main_control.getGlobalBounds().size.x / 2), padding_top + cover_size + offset + 60.f});

  auto next_tex = std::make_shared<sf::Texture>();
  if (!next_tex->loadFromFile(base_path_misc + "next.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "next.png'." << std::endl;
  }
  next_tex->setSmooth(true);

  // Skip to next song button
  sf::Sprite next_control(*next_tex);
  next_control.setPosition({main_control.getPosition().x + 40.f, main_control.getPosition().y});

  auto previous_tex = std::make_shared<sf::Texture>();
  if (!previous_tex->loadFromFile(base_path_misc + "previous.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "previous.png'." << std::endl;
  }
  previous_tex->setSmooth(true);

  // Skip to previous song button
  sf::Sprite previous_control(*previous_tex);
  previous_control.setPosition({main_control.getPosition().x - 40.f, main_control.getPosition().y});


  // Background behind the center "island" (big cover art, player controls and title + artist info)

  sf::RoundedRectangleShape player_background({cover_size + offset * 3, padding_top + cover_size + offset + 80.f}, out_round, main_n);
  player_background.setPosition({half - player_background.getSize().x / 2, padding_top - 30.f});
  player_background.setFillColor(background_color);

  sf::RoundedRectangleShape player_shadow_background({player_background.getGlobalBounds().size.x + shadow_offset, player_background.getGlobalBounds().size.y + shadow_offset}, out_round, main_n);
  player_shadow_background.setPosition({player_background.getPosition().x + shadow_offset, player_background.getPosition().y + shadow_offset});
  player_shadow_background.setFillColor(dark_main_color);


  // Progress bar

  float progress_width = third;

  sf::RoundedRectangleShape progress({progress_width, progress_height}, progress_round, progress_n);
  progress.setFillColor(progress_color);
  progress.setPosition({cover_size + (int)(offset / 2), padding_top + cover_size + offset + 40.f});

  sf::RoundedRectangleShape progress_shadow({progress_width, progress_height}, progress_round, progress_n);
  progress_shadow.setPosition({progress.getPosition().x + 5.f, progress.getPosition().y + 5.f});
  progress_shadow.setFillColor(background_shadow_color);

  auto live_empty_tex = std::make_shared<sf::Texture>();
  if (!live_empty_tex->loadFromFile(base_path_misc + "live_empty.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "live_empty.png'." << std::endl;
  }

  auto live_full_tex = std::make_shared<sf::Texture>();
  if (!live_full_tex->loadFromFile(base_path_misc + "live_full.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "live_full.png'." << std::endl;
  }

  // Live mode button
  sf::Sprite live(*live_empty_tex);
  live.setPosition({progress.getPosition().x + progress.getGlobalBounds().size.x - live.getGlobalBounds().size.x, main_control.getPosition().y});

  auto volume_tex = std::make_shared<sf::Texture>();
  if (!volume_tex->loadFromFile(base_path_misc + "volume.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "volume.png'." << std::endl;
  }
  volume_tex->setSmooth(true);

  auto mute_tex = std::make_shared<sf::Texture>();
  if (!mute_tex->loadFromFile(base_path_misc + "mute.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "mute.png'." << std::endl;
  }
  mute_tex->setSmooth(true);

  // Volume slider and icon next to the volume slider
  sf::Sprite vol_icon(*volume_tex);
  sf::RoundedRectangleShape vol_slider({100.f, progress_height - 2.f}, vol_round, vol_n);
  sf::RoundedRectangleShape vol_slider_shadow(vol_slider.getGlobalBounds().size, vol_round, vol_n);

  vol_icon.setPosition({progress.getPosition().x, main_control.getPosition().y});

  vol_slider.setPosition({vol_icon.getPosition().x + 30.f, vol_icon.getPosition().y + 11.f});
  vol_slider.setFillColor(progress_color);
  vol_slider_shadow.setPosition({vol_slider.getPosition().x + 2.f, vol_slider.getPosition().y + 3.f});
  vol_slider_shadow.setFillColor(background_shadow_color);


  // The controls in the upper rightish corner of the screen

  sf::RoundedRectangleShape control_corner({190.f, 50.f}, out_round, main_n);
  control_corner.setPosition({window_size.x - control_corner.getGlobalBounds().size.x - 20.f, -10.f});
  control_corner.setFillColor(background_color);

  sf::RoundedRectangleShape control_corner_shadow(control_corner.getGlobalBounds().size, out_round, main_n);
  control_corner_shadow.setPosition({control_corner.getPosition().x + shadow_offset, control_corner.getPosition().y + shadow_offset});
  control_corner_shadow.setFillColor(dark_main_color);


  // Queue

  sf::RoundedRectangleShape queue_background({500.f, window_size.y - 40.f}, out_round, main_n);
  queue_background.setPosition({queue_contracted_width - queue_background.getGlobalBounds().size.x, 20.f});
  queue_background.setFillColor(background_color);

  sf::RoundedRectangleShape queue_background_shadow(queue_background.getGlobalBounds().size, out_round, main_n);
  queue_background_shadow.setPosition({queue_background.getPosition().x + shadow_offset, queue_background.getPosition().y + shadow_offset});
  queue_background_shadow.setFillColor(dark_main_color);

  auto side_expand_tex = std::make_shared<sf::Texture>();
  if (!side_expand_tex->loadFromFile(base_path_misc + "side_expand.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "side_expand.png'." << std::endl;
  }
  side_expand_tex->setSmooth(true);

  auto side_contract_tex = std::make_shared<sf::Texture>();
  if (!side_contract_tex->loadFromFile(base_path_misc + "side_contract.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "side_contract.png'." << std::endl;
  }
  side_contract_tex->setSmooth(true);

  sf::Sprite queue_toggle(*side_expand_tex);
  queue_toggle.setPosition({0.f, queue_background.getPosition().y + 6.f});

  // Items in the control corner

  auto favorite_empty_tex = std::make_shared<sf::Texture>();
  if (!favorite_empty_tex->loadFromFile(base_path_misc + "favorite_empty.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "favorite_empty.png'." << std::endl;
  }
  favorite_empty_tex->setSmooth(true);

  auto favorite_full_tex = std::make_shared<sf::Texture>();
  if (!favorite_full_tex->loadFromFile(base_path_misc + "favorite_full.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "favorite_full.png'." << std::endl;
  }
  favorite_full_tex->setSmooth(true);

  sf::Sprite favorite(*favorite_empty_tex);
  favorite.setPosition({control_corner.getPosition().x + control_corner_gap, control_corner.getPosition().y + 12.f});

  auto manage_playlist_tex = std::make_shared<sf::Texture>();
  if (!manage_playlist_tex->loadFromFile(base_path_misc + "manage_playlist.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "manage_playlist.png'." << std::endl;
  }
  manage_playlist_tex->setSmooth(true);

  sf::Sprite manage_playlist(*manage_playlist_tex);
  manage_playlist.setPosition({favorite.getPosition().x + favorite.getGlobalBounds().size.x + control_corner_gap, control_corner.getPosition().y + 12.f});

  sf::Sprite playlist_selector(*manage_playlist_tex);
  playlist_selector.setPosition({queue_toggle.getPosition().x + 8.f, queue_toggle.getPosition().y + queue_toggle.getGlobalBounds().size.y + 6.f});

  auto trash_tex = std::make_shared<sf::Texture>();
  if (!trash_tex->loadFromFile(base_path_misc + "trash.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "trash.png'." << std::endl;
  }
  trash_tex->setSmooth(true);

  sf::Sprite trash(*trash_tex);
  trash.setPosition({manage_playlist.getPosition().x + manage_playlist.getGlobalBounds().size.x + control_corner_gap, control_corner.getPosition().y + 12.f});

  auto edit_tex = std::make_shared<sf::Texture>();
  if (!edit_tex->loadFromFile(base_path_misc + "edit.png")) {
    std::cerr << "Error: Failed to load '" << base_path_misc << "edit.png'." << std::endl;
  }
  edit_tex->setSmooth(true);

  sf::Sprite edit(*edit_tex);
  edit.setPosition({trash.getPosition().x + trash.getGlobalBounds().size.x + control_corner_gap, control_corner.getPosition().y + 12.f});

  // Little text at the bottom of the queue
  sf::Text playlist_data(default_font, "");
  playlist_data.setCharacterSize(20);
  playlist_data.setFillColor(light_text_color);
  playlist_data.setPosition({
    queue_contracted_width / 2 - 10.f,
    queue_background.getPosition().y + queue_background.getGlobalBounds().size.y - 30.f
  });


  auto general = init_general(
    window,
    {queue_background.getGlobalBounds().size.x - 100.f, 40.f},
    {50.f, queue_background.getPosition().y + 10.f}
  );


  auto spdata = std::make_unique<StaticPlayerData>(StaticPlayerData {
    main_control,
    next_control,
    previous_control,
    queue_toggle,
    favorite,
    manage_playlist,
    playlist_selector,
    trash,
    edit,
    vol_icon,
    live,
    general.cancel_search,
    artist,
    title,
    general.search_before_cursor,
    general.search_after_cursor,
    playlist_data,
    cover,
    cover_shadow,
    cover_texture,
    play_tex,
    pause_tex,
    cover_size,
    player_background,
    player_shadow_background,
    progress_width,
    progress,
    progress_shadow,
    next_tex,
    previous_tex,
    control_corner,
    control_corner_shadow,
    trash_tex,
    playlist,
    manage_playlist_tex,
    favorite_empty_tex,
    favorite_full_tex,
    edit_tex,
    queue_background,
    queue_background_shadow,
    general.search_background,
    true,
    side_expand_tex,
    side_contract_tex,
    false,
    volume_tex,
    mute_tex,
    vol_slider,
    vol_slider_shadow,
    false,
    live_full_tex,
    live_empty_tex,
    general.cancel_search_tex,
  });

  return spdata;
}

void display_player(MenuData::PlayerData& player, sf::RenderWindow& window) {
  auto& player_data = *player.data;
  auto& music = player.music;

  auto main_control = player_data.main_control; // Create a mutable copy of the main_control sprite

  auto playback_pos = music->get_playback_pos();
  if (playback_pos < slider_threshold) playback_pos = 0.015;

  sf::RoundedRectangleShape progress_done({player_data.progress_width * playback_pos, progress_height}, progress_round, progress_n);

  progress_done.setFillColor(main_color);
  progress_done.setPosition(player_data.progress.getPosition());

  sf::Text time_left(default_font, music->get_human_left_duration());
  time_left.setCharacterSize(20);
  time_left.setPosition({progress_done.getPosition().x + player_data.progress_width + 10.f, player_data.progress.getPosition().y - 10.f});
  time_left.setFillColor({66, 66, 66});

  if (music->is_playing()) main_control->setTexture(*player_data.pause_tex);
  else main_control->setTexture(*player_data.play_tex);

  auto volume = music->get_volume();
  sf::RoundedRectangleShape vol_slider_full({player_data.vol_slider.getGlobalBounds().size.x * volume, player_data.vol_slider.getGlobalBounds().size.y}, vol_round, vol_n);
  vol_slider_full.setPosition(player_data.vol_slider.getPosition());
  vol_slider_full.setFillColor({10, 10, 10});

  window.clear(main_color);

  window.draw(player_data.player_shadow_background);
  window.draw(player_data.player_background);
  window.draw(player_data.cover_shadow);
  window.draw(player_data.cover);
  window.draw(*player_data.artist);
  window.draw(*player_data.title);
  window.draw(player_data.progress_shadow);
  window.draw(player_data.progress);
  window.draw(progress_done);
  window.draw(time_left);
  window.draw(*main_control);
  window.draw(*player_data.next_control);
  window.draw(*player_data.previous_control);
  window.draw(*player_data.vol_icon);
  window.draw(player_data.vol_slider_shadow);
  window.draw(player_data.vol_slider);
  if (volume > slider_threshold) {
    window.draw(vol_slider_full);
  }
  window.draw(*player_data.live);

  window.draw(player_data.control_corner_shadow);
  window.draw(player_data.control_corner);
  window.draw(*player_data.trash);
  window.draw(*player_data.manage_playlist);
  window.draw(*player_data.favorite);
  window.draw(*player_data.edit);

  window.draw(player_data.queue_background_shadow);
  window.draw(player_data.queue_background);
  window.draw(*player_data.queue_toggle);
  if (player_data.queue_half_expanded) {
    window.draw(player_data.search_background);
    window.draw(*player_data.search_before_cursor);
    if (show_cursor) {
      search_draw_cursor(window, *player_data.search_before_cursor, player_data.search_background);
    }
    auto search_after_cursor = player_data.search_after_cursor; // Create a mutable copy of search_after_cursor
    search_after_cursor->setPosition({player_data.search_before_cursor->getPosition().x + player_data.search_before_cursor->getGlobalBounds().size.x + 2.8f, player_data.search_before_cursor->getPosition().y});
    window.draw(*search_after_cursor);
  }

  if (player_data.queue_expanded) {
    // Queue items

    sf::Texture queue_cover_texture;

    sf::RoundedRectangleShape queue_cover({queue_cover_size, queue_cover_size}, 8, main_n);

    sf::RoundedRectangleShape queue_cover_shadow(queue_cover.getGlobalBounds().size, 8, main_n);
    queue_cover_shadow.setFillColor(dark_background_shadow_color);

    sf::Text queue_title(default_font, "");
    queue_title.setFillColor(title_color);
    queue_title.setCharacterSize(18);

    sf::Text queue_artist(default_font, "");
    queue_artist.setFillColor(artist_color);
    queue_artist.setCharacterSize(18);

    sf::RoundedRectangleShape queue_entry_background({player_data.queue_background.getGlobalBounds().size.x - 25.f, queue_cover.getGlobalBounds().size.y + 10.f}, 8, main_n);
    queue_entry_background.setFillColor(background_shadow_color);

    sf::RoundedRectangleShape queue_entry_shadow(queue_entry_background.getGlobalBounds().size, 8, main_n);
    queue_entry_shadow.setFillColor(dark_background_shadow_color);

    sf::Text queue_entry_duration(default_font, "");
    queue_entry_duration.setFillColor(light_text_color);
    queue_entry_duration.setCharacterSize(18);

    sf::RoundedRectangleShape now_playing_bar({queue_entry_background.getGlobalBounds().size.x - 12.f, 4.f}, 2, main_n);
    now_playing_bar.setFillColor(main_color);

    sf::Texture queue_play_tex;
    if (!queue_play_tex.loadFromFile(base_path_misc + "queue_play.png")) {
      std::cerr << "Error: Failed to load '" << base_path_misc << "queue_play.png'." << std::endl;
    }
    queue_play_tex.setSmooth(true);

    sf::Sprite queue_play(queue_play_tex);

    auto get_queue_entry_position = [player_data](int index) {
      return player_data.search_background.getPosition().y + player_data.search_background.getGlobalBounds().size.y + 20.f + (queue_cover_size + 20.f) * index;
    };

    bool search_active = !search_string.empty();
    if (search_active) {
      window.draw(*player_data.cancel_queue_search);
    }

    // Dynamically set the attributes for these objects since player.queue can change at any time

    auto draw_ready_queue = player.queue;

    int new_idx = -1;

    // Move the queue entry being dragged to the bottom so it will be drawn above all other items
    if (player.dragging_queue != -1) {
      auto dragging_queue_iter = std::find(draw_ready_queue.begin(), draw_ready_queue.end(), player.dragging_queue);
      if (dragging_queue_iter != draw_ready_queue.end()) {
        draw_ready_queue.erase(dragging_queue_iter);
        draw_ready_queue.push_back(player.dragging_queue);
      }

      // Calculate the apparent index of the dragging queue entry
      auto relative_pos = (window.mapPixelToCoords(sf::Mouse::getPosition(window)).y - queue_entry_background.getGlobalBounds().size.y / 2) / player_data.queue_background.getGlobalBounds().size.y;
      if (relative_pos > 1) {
        std::cout << "TODO: Handle scrolling with dragging queue entry" << std::endl;
      }
      else {
        new_idx = round(relative_pos * (player_data.queue_background.getGlobalBounds().size.y / (queue_entry_background.getGlobalBounds().size.y + small_shadow_offset)));
        new_idx = std::clamp(new_idx, 1, static_cast<int>(draw_ready_queue.size()) - 1);
      }
    }

    // Move the current playing song to the top
    auto current_playing_iter = std::find(draw_ready_queue.begin(), draw_ready_queue.end(), player.song_id);
    if (current_playing_iter != draw_ready_queue.end()) {
      draw_ready_queue.erase(current_playing_iter);
      draw_ready_queue.insert(draw_ready_queue.begin(), player.song_id);
    }

    if (new_idx != -1) {
      draw_ready_queue.insert(draw_ready_queue.begin() + new_idx, -1); // Blank space
    }

    bool not_found = true;
    int idx = 0;
    for (const int id : draw_ready_queue) {
      if (id == -1) { // -1 means blank space
        idx += 1;
        if (idx >= queue_items) break;
        continue;
      }

      auto queue_song_path = construct_song_path(id);
      MusicPlayer queue_entry_player;
      queue_entry_player.load(queue_song_path + ".ogg");

      queue_cover.setScale({1, 1}); // Reset scale because of the hover effect

      sf::Image queue_cover_image;
      if (!queue_cover_image.loadFromFile(queue_song_path + ".small.png")) {
        std::cerr << "Error: Failed to load '" << queue_song_path << ".small.png" << "'." << std::endl;
      }

      if (id == player.dragging_queue) {
        for (unsigned int y = 0; y < queue_cover_image.getSize().y; y++) {
          for (unsigned int x = 0; x < queue_cover_image.getSize().x; x++) {
            auto pixel = queue_cover_image.getPixel({x, y});
            pixel.a = 128;

            queue_cover_image.setPixel({x, y}, pixel);
          }
        }
      }

      sf::Texture queue_cover_texture(queue_cover_image);
      queue_cover_texture.setSmooth(true);

      queue_cover.setTexture(&queue_cover_texture);
      queue_cover.setPosition({
        10.f,
        player.dragging_queue == id ?
          window.mapPixelToCoords(sf::Mouse::getPosition(window)).y - queue_entry_background.getGlobalBounds().size.y / 2:
          get_queue_entry_position(idx) // +1 makes space for the current playing song
      });

      std::ifstream artist_file(queue_song_path + ".artist");
      std::string artist_string = "";
      if (artist_file.good()) {
        std::getline(artist_file, artist_string);
      } else {
        std::cerr << "Error: Failed to read artist name from '" << queue_song_path << ".artist" << "'.";
      }

      if (artist_string.size() > queue_max_char) {
        artist_string.erase(queue_max_char - 3, artist_string.size());
        artist_string += "...";
      }

      std::ifstream title_file(queue_song_path + ".title");
      std::string title_string = "";
      if (title_file.good()) {
        std::getline(title_file, title_string);
      } else {
        std::cerr << "Error: Failed to read title from '" << queue_song_path << ".title" << "'.";
      }

      if (title_string.size() > queue_max_char) {
        title_string.erase(queue_max_char - 3, title_string.size());
        title_string += "...";
      }

      if (search_active && !matching(search_string, artist_string, match_diff) && !matching(search_string, title_string, match_diff)) continue; // Skip because it's not a match

      queue_title.setString(title_string);
      queue_title.setPosition({queue_cover.getPosition().x + queue_cover.getGlobalBounds().size.x + 5.f, queue_cover.getPosition().y + queue_cover_size / 3 - 10.f});

      queue_artist.setString(artist_string);
      queue_artist.setPosition({queue_title.getPosition().x, queue_title.getPosition().y + 20.f});

      queue_entry_background.setPosition({queue_cover.getPosition().x - 5.f, queue_cover.getPosition().y - 5.f});

      queue_entry_shadow.setPosition({queue_entry_background.getPosition().x + small_shadow_offset, queue_entry_background.getPosition().y + small_shadow_offset});

      queue_cover_shadow.setPosition({queue_cover.getPosition().x + 2.f, queue_cover.getPosition().y + 2.f});

      queue_entry_duration.setString(queue_entry_player.get_human_total_duration());
      queue_entry_duration.setPosition({
        queue_entry_background.getPosition().x + queue_entry_background.getGlobalBounds().size.x - queue_entry_duration.getGlobalBounds().size.x - 20.f,
        queue_entry_background.getPosition().y + queue_entry_background.getGlobalBounds().size.y / 2 - queue_entry_duration.getGlobalBounds().size.y / 2
      });

      if (id == player.song_id) {
        now_playing_bar.setPosition({
          queue_entry_background.getPosition().x + 6.f,
          queue_entry_background.getPosition().y + queue_entry_background.getGlobalBounds().size.y - now_playing_bar.getGlobalBounds().size.y
        });

        queue_entry_background.setFillColor(dark_background_shadow_color);
        queue_entry_shadow.setFillColor(background_shadow_color);
      }
      else if (id == player.dragging_queue) {
        queue_entry_background.setFillColor(background_shadow_color_transparent);
        queue_entry_shadow.setFillColor(dark_background_shadow_color_transparent);
      }
      else {
        queue_entry_background.setFillColor(background_shadow_color);
        queue_entry_shadow.setFillColor(dark_background_shadow_color);
      }

      // Hover checks

      auto mouse_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
      bool draw_queue_play = false;

      if (player.dragging_queue == -1) { // Only show hover effects when not dragging an item
        if (queue_entry_background.getGlobalBounds().contains(mouse_pos)) {
          queue_entry_duration.setString("\n...\n"); // Add the newlines to create a bigger clickable area
          queue_entry_duration.setFillColor(text_color);
          queue_entry_duration.move({0.f, -24.f});

          if (id != player.song_id) {
            draw_queue_play = true;
            queue_play.setPosition({
              queue_cover.getPosition().x + queue_cover.getGlobalBounds().size.x / 2 - queue_play.getGlobalBounds().size.x / 2,
              queue_cover.getPosition().y + queue_cover.getGlobalBounds().size.y / 2 - queue_play.getGlobalBounds().size.y / 2
            });

            queue_cover.setScale({0.45, 0.45}); // Scale is reset at the top
            queue_cover.move({24.f, 28.f});
          }
        }
        else {
          // Reset
          queue_entry_duration.setFillColor(light_text_color);
        }

        if (
            (draw_queue_play && queue_play.getGlobalBounds().contains(mouse_pos)) ||
            queue_entry_duration.getGlobalBounds().contains(mouse_pos)
          ) {

          window.setMouseCursor(hand_cursor);

          player.reset_cursor = false;
        }
      }

      if (player.dragging_queue != -1) {
        window.setMouseCursor(hand_cursor);

        player.reset_cursor = false;
      }

      // Click checks

      if (!held_left_mb_down && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        player.queue_play_pos = mouse_pos;

      if (draw_queue_play && player.queue_play_pos.x != -1 && queue_play.getGlobalBounds().contains(player.queue_play_pos)) { // Clicked on the play / cover art image
        player.song_id = id;
        player.queue_play_pos = {-1, -1};
      }
      else if (player.dragging_queue == -1 && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && queue_entry_duration.getGlobalBounds().contains(mouse_pos)) { // Clicked '...'
        std::cout << "TODO: Clicked ... menu in queue on id: " << id << std::endl;
      }
      else if (
          player.queue_play_pos.x != -1 &&
          id != player.song_id &&
          player.dragging_queue == -1 &&
          sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) &&
          queue_entry_background.getGlobalBounds().contains(player.queue_play_pos)
        ) {

        player.dragging_queue = id;
      }
      else {
        // Reset
        if (player.dragging_queue != -1 && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
          if (new_idx != -1) {
            player.queue.erase(std::find(player.queue.begin(), player.queue.end(), player.dragging_queue));
            player.queue.insert(player.queue.begin() + new_idx - 1, player.dragging_queue);
            std::cout << "new_idx: " << new_idx << std::endl;
            std::cout << "dragging_queue: " << player.dragging_queue << std::endl;
          }

          player.dragging_queue = -1;
        }
      }

      held_left_mb_down = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);


      window.draw(queue_entry_shadow);
      window.draw(queue_entry_background);
      if (!draw_queue_play) {
        window.draw(queue_cover_shadow);
      }
      window.draw(queue_entry_duration);
      window.draw(queue_cover);
      if (draw_queue_play) {
        window.draw(queue_play);
      }
      window.draw(queue_title);
      window.draw(queue_artist);


      not_found = false;
      idx++;
      if (idx >= queue_items) break; // Display a limited amount of queue (TODO: scrool)
    }

    if (not_found) { // Nothing was shown
      sf::Text nothing_exists(default_font, "");
      if (search_active) {
        nothing_exists.setString("No song was found");
      }
      else {
        nothing_exists.setString("Empty playlist");
      }

      nothing_exists.setFillColor(light_text_color);
      nothing_exists.setCharacterSize(18);
      nothing_exists.setPosition({player_data.queue_background.getGlobalBounds().size.x / 2 - nothing_exists.getGlobalBounds().size.x / 2, 100.f});

      window.draw(nothing_exists);
    }

    player_data.playlist_data->setString(player.playlist);
  }
  else {
    // Queue contracted

    player_data.playlist_data->setString(std::to_string(player.playlist.size() + 1));

    window.draw(*player_data.playlist_selector);
  }

  window.draw(*player_data.playlist_data);

  window.display();
}

void switch_to_player(MenuData& menu_data, std::string playlist) {
  menu_data.data = MenuData::PlayerData();
  menu_data.type = MenuData::Player;
  search_max_char = player_search_max_char;

  auto& pd = std::get<MenuData::PlayerData>(menu_data.data);

  pd.playlist = playlist;
  pd.queue = get_playlist(pd.playlist);
  pd.song_id = get_start_song(pd.queue);
  pd.is_valid = true;

  pd.data = init_player(window, construct_song_path(pd.song_id), playlist);
}
