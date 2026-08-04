#ifndef DATA_HPP
#define DATA_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <algorithm>
#include <random>
#include "../../external/lib/RoundedRectangleShape.hpp"
#include "utils.hpp"
#include "drawformable.hpp"

class InputComponent; // Forward declare

// Constants

extern const int ON_TOP;

extern const float padding_top;
extern const float offset;
extern const int in_round;
extern const int out_round;
extern const int main_n;
extern const float progress_height;
extern const int progress_round;
extern const int progress_n;
extern const int vol_round;
extern const int vol_n;
extern const float shadow_offset;
extern const float small_shadow_offset;
extern const float slider_threshold;
extern const float queue_cover_size;
extern const float selector_cover_size;
extern const sf::Vector2f selector_size;
extern const int queue_items;
extern const float move_speed;
extern const int match_diff;
extern const int player_search_max_char;
extern const int playlist_search_max_char;
extern int input_max_char;
extern const int queue_max_char;
extern const float queue_contracted_width;
extern const float control_corner_gap;
extern const float scroll_speed;

extern const float font_multiplier;
extern const float small_font_size;
extern const float medium_font_size;
extern const float medium_2_font_size;
extern const float large_font_size;

extern const sf::Vector2u window_base_size;
extern const sf::ContextSettings window_settings;
extern sf::RenderWindow window;
extern sf::Vector2f window_size;
extern sf::View default_view;
extern bool is_fullscreen;

extern int global_z_index;

extern const sf::Color main_color;
extern const sf::Color dark_main_color;
extern const sf::Color background_color;
extern const sf::Color dark_background_color;
extern const sf::Color light_background_color;
extern const sf::Color lighter_background_color;
extern const sf::Color background_shadow_color;
extern const sf::Color dark_background_shadow_color;
extern const sf::Color background_shadow_color_transparent;
extern const sf::Color dark_background_shadow_color_transparent;
extern const sf::Color progress_color;
extern const sf::Color progress_done_color;
extern const sf::Color text_color;
extern const sf::Color cursor_color;
extern const sf::Color light_text_color;
extern const sf::Color lighter_text_color;
extern const sf::Color white_color;
extern const sf::Color title_color;
extern const sf::Color artist_color;
extern const sf::Color selection_color;

extern const sf::Color hover_sub;

extern const sf::Cursor default_cursor;
extern const sf::Cursor text_cursor;
extern const sf::Cursor hand_cursor;

extern sf::Font default_font;

extern bool held_left_mb_down;
extern std::vector<int> search_results;
extern int dragging_search_result;
extern std::string progress_bar_string;
extern std::string progress_bar_doing_string;
extern float progress_bar_amount;
extern float progress_bar_total;
extern std::unique_ptr<std::thread> download_song_thread;
extern bool pause_main_input_handling;
extern float playlist_search_entry_height;
extern float playlist_search_entry_unit;
extern float playlist_search_scroll_lower_bound;
extern float playlist_sel_scroll;
extern bool can_search_string_scroll;
extern bool search_was_active;

extern std::random_device rd;
extern std::mt19937 rand_generator;

class MusicPlayer {
  public:
    sf::Music music;
    bool muted = false;
    bool was_muted = false;
    bool playing = false;
    bool started = false;
    float old_volume = music.getVolume();
    float apparent_volume = 1.f;

    bool load(const std::string& path) {
        if (!music.openFromFile(path)) {
            std::cerr << "Error: Failed to load music file for '" << path << "'." << std::endl;
            return false;
        }
        return true;
    }

    void play() {
      music.play();
      playing = true;
    }

    void pause() {
      music.pause();
      playing = false;
    }

    void toggle_play_state() {
      if (playing) this->pause();
      else this->play();
    }

    bool is_playing() const {
      return playing;
    }

    float get_playback_pos() const {
      return music.getPlayingOffset().asSeconds() / music.getDuration().asSeconds();
    }

    std::string get_human_left_duration() const {
      float total_sec_left = music.getDuration().asSeconds() - music.getPlayingOffset().asSeconds();

      return seconds_to_human_readable(total_sec_left);
    }

    std::string get_human_total_duration() const {
      return seconds_to_human_readable(music.getDuration().asSeconds());
    }

    void seek(float pos) {
      music.setPlayingOffset(pos * music.getDuration());
    }

    void mute() {
      apparent_volume = 0;
      muted = true;
      old_volume = music.getVolume();
      music.setVolume(0); // mute
    }

    void unmute() {
      apparent_volume = old_volume / 100;
      muted = false;
      if (old_volume) music.setVolume(old_volume);
      else music.setVolume(100); // fallback
    }

    void mute_while_seeking() {
      // don't set anything but the actual volume
      music.setVolume(0);
    }

    float get_volume() const {
      return apparent_volume;
    }

    void set_volume(float volume) {
      apparent_volume = volume;
      old_volume = volume * 100;
      music.setVolume(volume * 100);
    }

    void stop() {
      music.stop();
    }

    bool is_stopped() {
      if (music.getStatus() == sf::SoundSource::Status::Stopped) {
        if (started) return true;
        started = true; // 'started' is needed because the music starts as stopped and is
                        // effectively stopped twice, once at the start and once at the end
      }

      return false;
    }
};

struct StaticPlayerData {
  std::shared_ptr<InputComponent> search;
  std::optional<sf::Sprite> main_control;
  std::optional<sf::Sprite> next_control;
  std::optional<sf::Sprite> previous_control;
  std::optional<sf::Sprite> queue_toggle;
  std::optional<sf::Sprite> favorite;
  std::optional<sf::Sprite> manage_playlist;
  std::optional<sf::Sprite> playlist_selector;
  std::optional<sf::Sprite> trash;
  std::optional<sf::Sprite> edit;
  std::optional<sf::Sprite> vol_icon;
  std::optional<sf::Sprite> live;
  std::optional<sf::Text> artist;
  std::optional<sf::Text> title;
  std::optional<sf::Text> playlist_data;
  sf::RoundedRectangleShape cover;
  sf::RoundedRectangleShape cover_shadow;
  std::shared_ptr<sf::Texture> cover_texture;
  std::shared_ptr<sf::Texture> play_tex;
  std::shared_ptr<sf::Texture> pause_tex;
  float cover_size;
  sf::RoundedRectangleShape player_background;
  sf::RoundedRectangleShape player_shadow_background;
  float progress_width;
  sf::RoundedRectangleShape progress;
  sf::RoundedRectangleShape progress_shadow;
  std::shared_ptr<sf::Texture> next_tex;
  std::shared_ptr<sf::Texture> previous_tex;
  sf::RoundedRectangleShape control_corner;
  sf::RoundedRectangleShape control_corner_shadow;
  std::shared_ptr<sf::Texture> trash_tex;
  std::string playlist;
  std::shared_ptr<sf::Texture> manage_playlist_tex;
  std::shared_ptr<sf::Texture> favorite_empty_tex;
  std::shared_ptr<sf::Texture> favorite_full_tex;
  std::shared_ptr<sf::Texture> edit_tex;
  sf::RoundedRectangleShape queue_background;
  sf::RoundedRectangleShape queue_background_shadow;
  bool search_placeholder_active;
  std::shared_ptr<sf::Texture> side_expand_tex;
  std::shared_ptr<sf::Texture> side_contract_tex;
  bool queue_expanded;
  std::shared_ptr<sf::Texture> volume_tex;
  std::shared_ptr<sf::Texture> mute_tex;
  sf::RoundedRectangleShape vol_slider;
  sf::RoundedRectangleShape vol_slider_shadow;
  bool queue_half_expanded;
  std::shared_ptr<sf::Texture> live_full_tex;
  std::shared_ptr<sf::Texture> live_empty_tex;

  StaticPlayerData() = default;
  ~StaticPlayerData() = default;

  // Disallow copy
  StaticPlayerData(const StaticPlayerData&) = delete;
  StaticPlayerData& operator=(const StaticPlayerData&) = delete;

  // Allow move
  StaticPlayerData(StaticPlayerData&&) = default;
  StaticPlayerData& operator=(StaticPlayerData&&) = default;
};

struct StaticPlaylistSelectorData {
  std::shared_ptr<InputComponent> search;
  std::vector<std::string> playlists;
  DTCache drawables_cache;

  StaticPlaylistSelectorData() = default;
  ~StaticPlaylistSelectorData() = default;

  // Disallow copy
  StaticPlaylistSelectorData(const StaticPlaylistSelectorData&) = delete;
  StaticPlaylistSelectorData& operator=(const StaticPlaylistSelectorData&) = delete;

  // Allow move
  StaticPlaylistSelectorData(StaticPlaylistSelectorData&&) = default;
  StaticPlaylistSelectorData& operator=(StaticPlaylistSelectorData&&) = default;
};

struct MenuData {
  enum menu_types {
    Player,
    PlaylistSelector,
  } type;

  struct PlayerData {
    bool is_valid = false; // This has to be set to true to signify that the struct is ready to be used (all necessary fields are set)
    std::shared_ptr<MusicPlayer> music = std::make_shared<MusicPlayer>();
    std::shared_ptr<StaticPlayerData> data;
    int song_id;
    int playing_song_id = -1;
    bool seeking = false;
    bool was_playing = false;
    bool volume_slider_active = false;
    bool live_mode = false;
    int dragging_queue = -1;
    sf::Vector2f queue_play_pos;
    std::string song_path;
    std::string playlist;
    std::vector<int> queue;
    std::vector<int> past_queue;
    bool reset_cursor = true;
  };

  struct PlaylistSelectorData {
    bool is_valid = false; // Same as PlayerData::is_valid
    std::shared_ptr<StaticPlaylistSelectorData> data;
    bool reset_cursor = true;
    bool search_active = false;
    sf::Vector2f playlist_play_pos;
  };

  std::variant<
    MenuData::PlayerData,
    MenuData::PlaylistSelectorData
  > data;

  MenuData() : data(PlaylistSelectorData()) {} // Construct data with PlaylistSelectorData() so that clang is happy
};

#endif
