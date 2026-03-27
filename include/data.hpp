#ifndef DATA_HPP
#define DATA_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <iostream>
#include "RoundedRectangleShape.hpp"

// Constants

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
extern const std::string base_path;
extern const std::string base_path_misc;
extern const std::string base_path_external;
extern const std::string base_music_path;
extern const std::string base_music_path_data;
extern const std::string base_music_path_playlists;
extern const int queue_items;
extern const float move_speed;
extern const float match_diff;
extern const int player_search_max_char;
extern const int playlist_search_max_char;
extern int search_max_char;
extern const int queue_max_char;
extern const float queue_contracted_width;
extern const float control_corner_gap;

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
extern const sf::Color background_shadow_color_transparent;
extern const sf::Color dark_background_shadow_color_transparent;
extern const sf::Color progress_color;
extern const sf::Color text_color;
extern const sf::Color light_text_color;
extern const sf::Color white_color;
extern const sf::Color title_color;
extern const sf::Color artist_color;

extern const sf::Cursor default_cursor;
extern const sf::Cursor text_cursor;
extern const sf::Cursor hand_cursor;

extern bool search_active;
extern bool show_cursor;
extern size_t cursor_pos;
extern bool held_left_mb_down;
extern bool reset_cursor;
extern std::string search_string;

// Function definitions for data.cpp

std::string exec(const char* cmd);
std::vector<std::string> get_all_playlists();
std::vector<int> get_playlist(const std::string& name);
std::string construct_song_path(int id);
void done_playing(std::vector<int>& playlist, std::vector<int>& past);
std::string char32_to_utf8(char32_t c32);
bool matching(const std::string& s1, const std::string& s2, float diff, const char split = ' ');
int get_start_song(std::vector<int>& playlist);
std::string seconds_to_human_readable(float seconds);
bool download_song_from_query(const std::string& query);

// Adapted from:
// https://en.sfml-dev.org/forums/index.php?topic=24133.0

class DrawformableObject {
 public:
  DrawformableObject(std::shared_ptr<sf::Drawable> drawable, std::shared_ptr<sf::Transformable> transformable) {
    drawable_ = drawable;
    transformable_ = transformable;
  }

  sf::Drawable& get_drawable() {
    return *drawable_;
  }

  sf::Transformable& get_transformable() {
    return *transformable_;
  }

  // Very unsafe getGlobalBounds implementation, requires drawable_ to be a sf::Transformable object
  sf::FloatRect getGlobalBounds() const {
    // Try Sprite
    if (auto sprite = dynamic_cast<sf::Sprite*>(drawable_.get())) {
        return transformable_->getTransform().transformRect(sprite->getLocalBounds());
    }

    // Try Shape (CircleShape, RectangleShape, etc.)
    if (auto shape = dynamic_cast<sf::Shape*>(drawable_.get())) {
        return transformable_->getTransform().transformRect(shape->getLocalBounds());
    }

    // Try Text
    if (auto text = dynamic_cast<sf::Text*>(drawable_.get())) {
        return transformable_->getTransform().transformRect(text->getLocalBounds());
    }

    throw std::runtime_error("getGlobalBounds() not supported for this drawable type");
  }

private:
  std::shared_ptr<sf::Drawable> drawable_;
  std::shared_ptr<sf::Transformable> transformable_;
};

// ====================================================================

struct DTPair {
  std::shared_ptr<DrawformableObject> drawformable;
  std::shared_ptr<sf::Texture> texture;
};

struct DTCache {
  std::vector<int> ids;
  std::vector<std::vector<std::string>> names;
  std::vector<std::vector<DTPair>> items;

  size_t find(int id) {
    return std::distance(ids.begin(), std::find(ids.begin(), ids.end(), id));
  }

  int name_to_z_index(int id, const std::string& name) {
    auto idx = this->find(id);
    auto& names_vec = names[idx];

    return std::distance(names_vec.begin(), std::find(names_vec.begin(), names_vec.end(), name));
  }

  bool contains(int id) {
    return (ids.begin() + this->find(id)) != ids.end();
  }

  void add(int id, std::string name, const DTPair& dt) {
    if (this->contains(id)) {
      auto idx = this->find(id);
      auto& vec = items[idx];
      vec.push_back(dt);
      names[idx].push_back(name);
    }
    else {
      ids.push_back(id);
      items.push_back({dt});
      names.push_back({name});
    }
  }

  void remove(int id) {
    auto idx = this->find(id);

    ids.erase(ids.begin() + idx);
    items.erase(items.begin() + idx);
  }

  void draw(int id, sf::RenderWindow& window) {
    auto& vec = items[this->find(id)];

    for (auto dt : vec) {
      window.draw(dt.drawformable->get_drawable());
    }
  }

  DTPair& get_item(int id, int z_index) {
    return items[this->find(id)][z_index];
  }
};

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

struct StaticData {
  sf::RoundedRectangleShape search_background;
  sf::RoundedRectangleShape search_shadow;
  sf::Text search_before_cursor;
  sf::Text search_after_cursor;
  std::shared_ptr<sf::Texture> cancel_search_tex;
  sf::Sprite cancel_search;
};

struct StaticPlayerData {
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
  std::optional<sf::Sprite> cancel_queue_search;
  std::optional<sf::Text> artist;
  std::optional<sf::Text> title;
  std::optional<sf::Text> search_before_cursor;
  std::optional<sf::Text> search_after_cursor;
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
  sf::RoundedRectangleShape search_background;
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
  std::shared_ptr<sf::Texture> cancel_queue_search_tex;
};

struct StaticPlaylistSelectorData {
  sf::RoundedRectangleShape search_background;
  sf::RoundedRectangleShape search_shadow;
  sf::Text search_before_cursor;
  sf::Text search_after_cursor;
  std::shared_ptr<sf::Texture> cancel_search_tex;
  sf::Sprite cancel_search;
  sf::RoundedRectangleShape control_corner;
  sf::RoundedRectangleShape control_corner_shadow;
  std::shared_ptr<sf::Texture> add_playlist_tex;
  sf::Sprite add_playlist;
  std::shared_ptr<sf::Texture> playlist_play_tex;
  sf::Sprite playlist_play;
  std::vector<std::string> playlists;
  DTCache drawables_cache;
};

struct MenuData {
  enum menu_types {
    Player,
    PlaylistSelector,
  } type;

  struct PlayerData {
    bool is_valid = false; // This has to be set to true to signify that the struct is ready to be used (all necessary fields are set)
    std::unique_ptr<MusicPlayer> music = std::make_unique<MusicPlayer>();
    std::unique_ptr<StaticPlayerData> data;
    int song_id; // Needs to be set manually
    int playing_song_id = -1;
    bool seeking = false;
    bool was_playing = false;
    bool volume_slider_active = false;
    bool live_mode = false;
    int dragging_queue = -1;
    sf::Vector2f queue_play_pos;
    std::string song_path;
    std::string playlist; // Needs to be set manually
    std::vector<int> queue;
    std::vector<int> past_queue;
    bool reset_cursor = true;
  };

  struct PlaylistSelectorData {
    bool is_valid = false; // Same as PlayerData is_valid
    std::unique_ptr<StaticPlaylistSelectorData> data;
    bool reset_cursor = true;
    bool search_active = false;
    sf::Vector2f playlist_play_pos;
  };

  std::variant<
    MenuData::PlayerData,
    MenuData::PlaylistSelectorData
  > data;
};

enum class AnimationStage {
  start,
  half,
  end,
};

#endif
