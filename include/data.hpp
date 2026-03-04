#ifndef DATA_HPP
#define DATA_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <iostream>
#include "RoundedRectangleShape.hpp"

// Constants

const sf::Font default_font("misc/JetBrainsMono-Regular.ttf");

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
extern const std::string base_path;
extern const int queue_items;
extern const float move_speed;
extern const float match_diff;
extern const int queue_search_max_char;
extern const int queue_max_char;

extern const sf::Vector2u window_base_size;
extern const sf::ContextSettings settings;
extern sf::RenderWindow window;
extern const sf::Vector2u window_size;

extern const sf::Color main_color;
extern const sf::Color dark_main_color;
extern const sf::Color background_color;
extern const sf::Color dark_background_color;
extern const sf::Color background_shadow_color;
extern const sf::Color dark_background_shadow_color;
extern const sf::Color progress_color;
extern const sf::Color text_color;
extern const sf::Color light_text_color;
extern const sf::Color white_color;
extern const sf::Color title_color;
extern const sf::Color artist_color;

extern const sf::Cursor default_cursor;
extern const sf::Cursor text_cursor;
extern const sf::Cursor hand_cursor;

// Function definitions for data.cpp

std::vector<int> get_playlist(const std::string& name);
std::string construct_song_path(const std::string& playlist, int id);
void done_playing(std::vector<int>& playlist, std::vector<int>& past);
std::string char32_to_utf8(char32_t c32);
bool matching(const std::string& s1, const std::string& s2, float diff, const char split = ' ');
int get_start_song(std::vector<int>& playlist);
std::string seconds_to_human_readable(float seconds);

class MusicPlayer {
  public:
    sf::Music music;
    bool muted = false;
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

    void silent_mute() {
      // don't set apparent_volume
      muted = true;
      old_volume = music.getVolume();
      music.setVolume(0); // mute
    }

    float get_volume() const {
      return apparent_volume;
    }

    void set_volume(float volume) {
      apparent_volume = volume;
      music.setVolume(volume * 100);
    }

    void stop() {
      music.stop();
    }

    bool is_stopped() {
      if (music.getStatus() == sf::SoundSource::Status::Stopped) {
        if (started) return true;
        started = true; // started is needed because the music starts as stopped and is
                        // effectively stopped twice, once at the start and once at the end
      }

      return false;
    }
};

struct StaticPlayerData {
  sf::RoundedRectangleShape cover;
  sf::RoundedRectangleShape cover_shadow;
  sf::Sprite main_control;
  sf::Texture cover_texture;
  sf::Texture play_tex;
  sf::Texture pause_tex;
  float cover_size;
  sf::Text artist;
  sf::Text title;
  sf::RoundedRectangleShape player_background;
  sf::RoundedRectangleShape player_shadow_background;
  float progress_width;
  sf::RoundedRectangleShape progress;
  sf::RoundedRectangleShape progress_shadow;
  std::shared_ptr<sf::Texture> next_tex;
  sf::Sprite next_control;
  std::shared_ptr<sf::Texture> previous_tex;
  sf::Sprite previous_control;
  sf::RoundedRectangleShape control_corner;
  sf::RoundedRectangleShape control_corner_shadow;
  std::shared_ptr<sf::Texture> trash_tex;
  sf::Sprite trash;
  std::string playlist;
  std::shared_ptr<sf::Texture> manage_playlist_tex;
  sf::Sprite manage_playlist;
  std::shared_ptr<sf::Texture> favorite_empty_tex;
  std::shared_ptr<sf::Texture> favorite_full_tex;
  sf::Sprite favorite;
  std::shared_ptr<sf::Texture> edit_tex;
  sf::Sprite edit;
  sf::RoundedRectangleShape queue_background;
  sf::RoundedRectangleShape queue_background_shadow;
  sf::RoundedRectangleShape search_background;
  sf::Text search_before_cursor;
  sf::Text search_after_cursor;
  bool search_placeholder_active;
  std::shared_ptr<sf::Texture> side_expand_tex;
  std::shared_ptr<sf::Texture> side_contract_tex;
  sf::Sprite queue_toggle;
  bool queue_expanded;
  std::shared_ptr<sf::Texture> volume_tex;
  std::shared_ptr<sf::Texture> mute_tex;
  sf::Sprite vol_icon;
  sf::RoundedRectangleShape vol_slider;
  sf::RoundedRectangleShape vol_slider_shadow;
  bool queue_half_expanded;
  std::shared_ptr<sf::Texture> live_full_tex;
  std::shared_ptr<sf::Texture> live_empty_tex;
  sf::Sprite live;
  std::string search_string;
  std::shared_ptr<sf::Texture> cancel_queue_search_tex;
  sf::Sprite cancel_queue_search;
};

struct MenuData {
  enum menu_types {
    Player,
    Main,
    Selector,
  } type;

  struct PlayerData {
      MusicPlayer music;
      std::optional<StaticPlayerData> data;
      int song_id; // Needs to be set manually
      int playing_song_id = -1;
      bool seeking = false;
      bool was_playing = false;
      bool volume_slider_active = false;
      bool search_active = false;
      bool show_cursor = false;
      size_t cursor_pos = 0;
      bool live_mode = false;
      bool held_left_mb_down = false;
      sf::Vector2f queue_play_pos;
      std::string song_path;
      std::string playlist; // Needs to be set manually
      std::vector<int> queue;
      std::vector<int> past_queue;
  };

  std::variant<MenuData::PlayerData> data;
};

enum class AnimationStage {
  start,
  half,
  end,
};

#endif
