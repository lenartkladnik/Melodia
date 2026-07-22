#ifndef DATA_HPP
#define DATA_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <unordered_map>
#include "../../external/lib/RoundedRectangleShape.hpp"

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
extern const std::string base_path_external_prog;
extern const std::string base_music_path;
extern const std::string base_music_path_data;
extern const std::string base_music_path_playlists;
extern const int queue_items;
extern const float move_speed;
extern const float match_diff;
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
extern const sf::ContextSettings settings;
extern sf::RenderWindow window;
extern sf::Vector2f window_size;
extern sf::View default_view;

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

extern const sf::Color hover_sub;

extern const sf::Cursor default_cursor;
extern const sf::Cursor text_cursor;
extern const sf::Cursor hand_cursor;

extern sf::Font default_font;

extern bool held_left_mb_down;
extern std::vector<int> search_results;
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

class PopupComponent;
extern std::unordered_map<std::string, std::shared_ptr<PopupComponent>> popup_components;

// Function definitions for data.cpp

void new_random();
std::string exec(const char* cmd);
std::string construct_song_path(int id);
std::string get_song_title(int id);
std::string get_song_artist(int id);
std::vector<std::string> get_all_playlists();
std::vector<int> get_playlist(const std::string& name);
void done_playing(std::vector<int>& playlist, std::vector<int>& past);
std::string char32_to_utf8(char32_t c32);
bool matching(const std::string& s1, const std::string& s2, float diff, const char split = ' ');
int get_start_song(std::vector<int>& playlist);
std::string seconds_to_human_readable(float seconds);
std::vector<int> search_all_songs(const std::string& query);
float getFontOffsetPixels(float target_size);
void setFontSize(sf::Text& text, float target_size, unsigned int raster_mul = 2);
void reset_globals();

enum class AnimationStage {
  start,
  half,
  end,
};

class UIComponent; // Forward declare UIComponent so the events can use it

class MenuData; // Forward declare MenuData so ClickEvent can use it

struct UIEvent {
  std::string id;
  sf::FloatRect bounds;
  sf::View view = default_view;
  UIComponent* component = nullptr;
};

struct ClickEvent : UIEvent {
  std::function<void(MenuData&)> function;
  sf::Mouse::Button mouse_button;
};

extern std::vector<ClickEvent> click_events;
extern std::vector<ClickEvent> search_res_click_events;
void new_click_event(
  std::vector<ClickEvent>& container,
  std::string id,
  std::function<void(MenuData&)> function,
  sf::FloatRect bounds,
  sf::Mouse::Button mouse_button,
  UIComponent* component = nullptr,
  sf::View view = default_view
);

struct HoverEvent : UIEvent {};

extern std::vector<HoverEvent> hover_events;
void new_hover_event(
  std::vector<HoverEvent>& container,
  std::string id,
  sf::FloatRect bounds,
  UIComponent* component = nullptr,
  sf::View view = default_view
);

struct FocusEvent : UIEvent {
  std::function<void(MenuData&, sf::Vector2f&)> function;
  std::function<void(MenuData&)> else_function;
  sf::Mouse::Button mouse_button;
};

extern std::vector<FocusEvent> focus_events;
void new_focus_event(
  std::vector<FocusEvent>& container,
  std::string id,
  std::function<void(MenuData&, sf::Vector2f&)> function, // Will get called if the click is within bounds
  std::function<void(MenuData&)> else_function, // Will get called if click is out of bounds
  sf::FloatRect bounds,
  sf::Mouse::Button mouse_button,
  UIComponent* component = nullptr,
  sf::View view = default_view
);

struct ScrollEvent : UIEvent {
  float& scroll_offset;
  bool& can_scroll;
};

extern std::vector<ScrollEvent> scroll_events;
void new_scroll_event(
  std::vector<ScrollEvent>& container,
  std::string id,
  sf::FloatRect bounds,
  float& scroll_offset,
  bool& can_scroll,
  UIComponent* component = nullptr
);

class InputComponent; // Forward declare InputComponent so TextEvent can use it

struct TextEvent : UIEvent {
  InputComponent* input_component;
};

extern std::vector<TextEvent> text_events;
void new_text_event(std::vector<TextEvent>& container, std::string id, InputComponent* input_component, UIComponent* component = nullptr);


struct KbEvent : UIEvent {};

extern std::vector<KbEvent> kb_events;
void new_kb_event(std::vector<KbEvent>& container, std::string id, UIComponent* component = nullptr);

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

  // Unsafe getGlobalBounds implementation, requires drawable_ to be a sf::Transformable object
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
    names.erase(names.begin() + idx);
    items.erase(items.begin() + idx);
  }

  void clear() {
    ids.clear();
    names.clear();
    items.clear();
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

class UIComponent {
  public:
    std::string id;
    int z_index;
    bool m_hidden = false;
    bool m_focused = false;
    bool m_hover = false;

    virtual void draw() {};

    virtual void focus() {
      m_focused = true;
    }

    virtual void unfocus() {
      m_focused = false;
    }

    virtual bool is_focused() {
      return m_focused;
    }

    virtual void hide() {
      m_hidden = true;
    }

    virtual void show() {
      m_hidden = false;
    }

    virtual bool is_hidden() {
      return m_hidden;
    }

    virtual void on_hover() {
      m_hover = true;
    }

    virtual void off_hover() {
      m_hover = false;
    }

    UIComponent(std::string id, bool hidden = false)
      : id(std::move(id))
    {
      z_index = global_z_index;
      global_z_index++;

      if (hidden)
        this->hide();
    }

    virtual ~UIComponent() = default;
};

class InputComponent : public UIComponent {
  private:
    std::string input_string = "";
    size_t cursor_pos = 0;
    sf::RenderWindow& window;
    bool show_cursor = false;
    bool reset_cursor_flag = false;
    bool input_active = false;
    bool refresh_input_flag = true;
    sf::Clock cursor_clock;
    std::string prev_input_string = "";
    std::string input_prompt;
    float corner_radius;
    sf::RoundedRectangleShape input_background;
    sf::RoundedRectangleShape input_shadow;
    sf::Text input_before_cursor;
    sf::Text input_after_cursor;
    std::shared_ptr<sf::Texture> action_button_tex;
    std::optional<sf::Sprite> action_button;

  public:
    void draw() override {
      if (!this->is_hidden()) {
        window.draw(input_background);
        window.draw(input_before_cursor);
        window.draw(input_after_cursor);
        if (action_button)
          window.draw(*action_button);
        if (show_cursor)
          this->draw_cursor();
      }
    }

    void reset_cursor() {
      cursor_clock.restart();
      show_cursor = true;
    }

    void move_cursor_left() {
      if (cursor_pos > 0) {
        // Keep cursor solid while changing cursor pos
        this->reset_cursor();

        cursor_pos--;
      }
    }

    void move_cursor_right() {
      if (cursor_pos < input_string.size()) {
        // Keep cursor solid while changing cursor pos
        this->reset_cursor();

        cursor_pos++;
      }
    }

    void update() {
      if (cursor_clock.getElapsedTime() >= sf::milliseconds(500)) {
        cursor_clock.restart();
        show_cursor = !show_cursor;
      }

      input_before_cursor.setString(input_string.substr(0, cursor_pos));
      input_after_cursor.setString(input_string.substr(cursor_pos));
    }

    void write_input(char32_t input) {
      // Keep cursor solid while inputting
      this->reset_cursor();

      if (input < 32 || input > 126) {
        switch (input) {
          case 8:
            if (input_string.size() > 0) {
              cursor_pos--;
              input_string.erase(cursor_pos, 1);
            }
            break;
        }
      }
      else {
        input_string.insert(cursor_pos, char32_to_utf8(input));
        cursor_pos++;
        this->update();
        if (this->is_text_too_long()) {
          cursor_pos--;
          input_string.erase(cursor_pos, 1);
        }
        this->update();
      }
    }

    void focus(const sf::Vector2f& pos) {
      this->m_focused = true;

      input_before_cursor.setFillColor(text_color);

      if (input_active) {
        if (input_string.empty()) {
          cursor_pos = 0;
        }
        else {
          auto relative_before_cursor = (pos.x - input_before_cursor.getPosition().x);
          auto c_pos = round(relative_before_cursor / (input_before_cursor.getGlobalBounds().size.x + input_after_cursor.getGlobalBounds().size.x) * input_string.size());

          // Keep cursor solid while changing cursor pos
          this->reset_cursor();

          if (c_pos > input_string.size()) {
            cursor_pos = input_string.size();
          }
          else {
            cursor_pos = c_pos;
          }
        }
      }
      else {
        input_active = true;
        input_before_cursor.setString(input_string.substr(0, cursor_pos));
        input_after_cursor.setString(input_string.substr(cursor_pos));
      }
    }

    void unfocus() {
      this->m_focused = false;

      input_active = false;
      show_cursor = false;

      if (input_string.size() == 0) {
        input_after_cursor.setFillColor(text_color);
        input_before_cursor.setFillColor(light_text_color);
        input_before_cursor.setString(input_prompt);
      }
    }

    bool is_text_too_long() {
      float action_button_size_x = 0.f;

      if (action_button.has_value()) {
        action_button_size_x = action_button.value().getGlobalBounds().size.x + 10.f; // + 10 so the text and action_button aren't touching
      }

      return (
        input_before_cursor.getGlobalBounds().size.x +
        input_after_cursor.getGlobalBounds().size.x) // Full size of the text
        >
        input_background.getGlobalBounds().size.x -
        (input_before_cursor.getPosition().x - input_background.getPosition().x) - // Subtract the offset of the text at the start
        action_button_size_x; // Subtract the size of the action button
    }

    bool is_active() {
      return input_active;
    }

    bool not_empty() {
      return !input_string.empty();
    }

    bool should_input_refresh() {
      return refresh_input_flag || prev_input_string != input_string;
    }

    void force_input_refresh() {
      refresh_input_flag = true;
    }

    void input_refresh() {
      prev_input_string = input_string;
      refresh_input_flag = false;
    }

    void clear_input() {
      input_string = "";
      cursor_pos = 0;
    }

    std::string& get_input_string() {
      return input_string;
    }

    void draw_cursor() {
      sf::RectangleShape cursor({1.2, input_background.getGlobalBounds().size.y - 15.f});
      cursor.setPosition({input_before_cursor.getPosition().x + 2.f + input_before_cursor.getGlobalBounds().size.x, input_background.getPosition().y + 6.f}); // input_before_cursor.getPosition().y + 1.f});
      cursor.setFillColor(cursor_color);
      window.draw(cursor);
    }

    void draw_input_shadow() {
      // window.draw(input_shadow);
    }

    sf::FloatRect background_bounds() {
      return input_background.getGlobalBounds();
    }

    sf::Vector2f background_pos() {
      return input_background.getPosition();
    }

    float background_get_corner_radius(size_t index) {
      return input_background.getCornersRadius(index);
    }

    void background_set_corner_radii(std::array<float, 4> radii) {
      input_background.setCornerRadii(radii);
    }

    void background_reset_corner_radii() {
      input_background.setCornersRadius(corner_radius);
    }

    sf::FloatRect action_button_bounds() {
      if (action_button)
        return action_button->getGlobalBounds();

      sf::FloatRect dummy_rect;
      dummy_rect.position = sf::Vector2f(0, 0);
      dummy_rect.size = sf::Vector2f(0, 0);
      return dummy_rect;
    }

    void register_action(std::function<void(MenuData&)> action_function) {
      if (action_button) {
        new_click_event(click_events, id + "_action_button", [action_function](MenuData& menu_data) {
          action_function(menu_data);
        }, action_button->getGlobalBounds(), sf::Mouse::Button::Left, this);
      }
    }

    InputComponent(
      sf::RenderWindow& render_window,
      std::string id,
      sf::Vector2f input_size,
      sf::Vector2f input_pos,
      std::string prompt,
      std::shared_ptr<sf::Texture> action_tex,
      std::function<void(MenuData&)> action_function,
      float corner_radius = 20,
      bool hidden = false
    )
      : UIComponent(id, hidden),
        window(render_window),
        input_prompt(prompt),
        corner_radius(corner_radius),
        input_before_cursor(default_font, prompt, 20),
        input_after_cursor(default_font, "", 20)

    {
      if (action_tex) {
        action_button_tex = action_tex;
        action_button.emplace(*action_button_tex);
      }

      input_string = "";
      cursor_pos = 0;

      input_background.setSize(input_size);
      input_background.setCornerPointCount(main_n);
      input_background.setPosition(input_pos);
      input_background.setFillColor(light_background_color);
      background_reset_corner_radii();

      input_shadow.setSize({input_background.getGlobalBounds().size.x + 5.f, input_background.getGlobalBounds().size.y + 5.f});
      input_shadow.setCornersRadius(corner_radius);
      input_shadow.setCornerPointCount(main_n);
      input_shadow.setPosition({input_background.getPosition().x - 2.5f, input_background.getPosition().y + 3.f});
      input_shadow.setFillColor(dark_main_color);

      input_before_cursor.setFillColor(light_text_color);
      setFontSize(input_before_cursor, medium_font_size);
      input_before_cursor.setPosition({input_background.getPosition().x + 16.f, input_background.getPosition().y + input_before_cursor.getGlobalBounds().size.y - 2.5});

      input_after_cursor.setFillColor(text_color);
      setFontSize(input_after_cursor, medium_font_size);
      input_after_cursor.setPosition(input_before_cursor.getPosition());

      if (action_button) {
        action_button->setPosition({
          input_background.getPosition().x + input_background.getGlobalBounds().size.x - action_button->getGlobalBounds().size.x - 12.f,
          input_background.getPosition().y + 5.f
        });

        register_action(action_function);
      }

      new_focus_event(focus_events, id,
        [this](MenuData& menu_data, sf::Vector2f pos) {
          if (!this->is_hidden())
            this->focus(pos);
        },
        [this](MenuData& menu_data) {
          this->unfocus();
        },
        input_background.getGlobalBounds(), sf::Mouse::Button::Left, this);

      new_text_event(text_events, id, this, this);
    }


  InputComponent() = delete;
  ~InputComponent() = default;

  // Disallow copy
  InputComponent(const InputComponent&) = delete;
  InputComponent& operator=(const InputComponent&) = delete;

  // Disallow move
  InputComponent(InputComponent&&) = delete;
  InputComponent& operator=(InputComponent&&) = delete;
};

class ButtonComponent : public UIComponent {
  // TODO:
  // - Add pressed down styling

  private:
    sf::RenderWindow& window;
    sf::RoundedRectangleShape button_shape;
    sf::Text button_text;
    std::function<void(MenuData&)> function;
    sf::Color m_button_shape_color;

  public:
    void draw() override {
      if (!this->is_hidden()) {
        window.draw(button_shape);
        window.draw(button_text);
      }
    }

    void on_hover() override {
      m_hover = true;

      button_shape.setFillColor(m_button_shape_color - hover_sub);
    }

    void off_hover() override {
      m_hover = false;

      button_shape.setFillColor(m_button_shape_color);
    }

    sf::FloatRect button_bounds() {
      return button_shape.getGlobalBounds();
    }

    ButtonComponent(
      sf::RenderWindow& render_window,
      std::string id,
      std::string text,
      sf::Vector2f size,
      sf::Vector2f pos,
      std::function<void(MenuData&)> function,
      bool hidden = false,
      sf::Color button_shape_color = dark_background_color,
      sf::Color button_text_color = text_color,
      int corner_radius = 10
    )
      : UIComponent(id, hidden),
        window(render_window),
        button_text(default_font, text, 0),
        function(function),
        m_button_shape_color(button_shape_color)
    {
      button_shape.setSize(size);
      button_shape.setPosition(pos);
      button_shape.setFillColor(button_shape_color);
      button_shape.setCornersRadius(corner_radius);
      button_shape.setCornerPointCount(main_n);

      int text_size = 0;
      if (text.size() != 0)
        text_size = (int)((size.x/text.size()) * font_multiplier);

      setFontSize(button_text, text_size);
      button_text.setFillColor(button_text_color);
      button_text.setPosition({
        button_shape.getPosition().x + button_shape.getGlobalBounds().size.x / 2 - button_text.getGlobalBounds().size.x / 2,
        button_shape.getPosition().y + button_shape.getGlobalBounds().size.y / 2 - button_text.getGlobalBounds().size.y
      });

      new_click_event(click_events, id, [this, function](MenuData& menu_data) { if (!this->is_hidden()) function(menu_data); }, button_shape.getGlobalBounds(), sf::Mouse::Button::Left, this);
    }

    ~ButtonComponent() = default;

  // Disallow copy
  ButtonComponent(const ButtonComponent&) = delete;
  ButtonComponent& operator=(const ButtonComponent&) = delete;

  // Disallow move
  ButtonComponent(ButtonComponent&&) = delete;
  ButtonComponent& operator=(ButtonComponent&&) = delete;
};

class PopupComponent : public UIComponent {
  private:
    std::unordered_map<std::string, std::shared_ptr<InputComponent>> input_components;
    std::unordered_map<std::string, std::shared_ptr<ButtonComponent>> button_components;
    std::unordered_map<std::string, sf::RoundedRectangleShape> rounded_rectangle_shapes;

  public:
    void draw() override {
      if (!this->is_hidden()) {
        draw_rounded_rectangle_shapes();
        draw_input_components();
        draw_button_components();
      }
    }

    std::shared_ptr<InputComponent> get_input(std::string id) {
      for (auto it = input_components.begin(); it != input_components.end(); it++) {
        if (it->first == id)
          return it->second;
      }

      return nullptr;
    }

    std::shared_ptr<ButtonComponent> get_button(std::string id) {
      for (auto it = button_components.begin(); it != button_components.end(); it++) {
        if (it->first == id)
          return it->second;
      }

      return nullptr;
    }

    void new_input(std::string input_id, std::shared_ptr<InputComponent> input_component) {
      input_component->z_index += this->z_index;

      input_components.insert({input_id, input_component});
    }

    void new_button(std::string id, std::shared_ptr<ButtonComponent> button_component) {
      button_component->z_index += this->z_index;

      button_components.insert({id, std::move(button_component)});
    }

    void new_rounded_rectangle_shape(std::string id, sf::RoundedRectangleShape rounded_rectangle_shape) {
      rounded_rectangle_shapes.insert({id, rounded_rectangle_shape});
    }

    void draw_input_components() {
      for (auto it = input_components.begin(); it != input_components.end(); it++) {
        it->second->draw();
      }
    }

    void draw_button_components() {
      for (auto it = button_components.begin(); it != button_components.end(); it++) {
        it->second->draw();
      }
    }

    void draw_rounded_rectangle_shapes() {
      for (auto it = rounded_rectangle_shapes.begin(); it != rounded_rectangle_shapes.end(); it++) {
        window.draw(it->second);
      }
    }

    void hide() override {
      m_hidden = true;

      for (auto it = input_components.begin(); it != input_components.end(); it++) {
        it->second->hide();
      }

      for (auto it = button_components.begin(); it != button_components.end(); it++) {
        it->second->hide();
      }
    }

    void show() override {
      m_hidden = false;

      for (auto it = input_components.begin(); it != input_components.end(); it++) {
        it->second->show();
      }

      for (auto it = button_components.begin(); it != button_components.end(); it++) {
        it->second->show();
      }
    }

    static PopupComponent* Create(std::string id) {
      auto instance = std::make_shared<PopupComponent>(id);
      PopupComponent* ptr = instance.get();

      popup_components.insert({id, std::move(instance)});

      return ptr;
    }

    PopupComponent(std::string id)
      : UIComponent(id) {}

    ~PopupComponent() = default; // Popup id gets erased by itself which causes the pointers to be freed

  // Disallow copy
  PopupComponent(const PopupComponent&) = delete;
  PopupComponent& operator=(const PopupComponent&) = delete;

  // Allow move
  PopupComponent(PopupComponent&&) = default;
  PopupComponent& operator=(PopupComponent&&) = default;
};

class AreaComponent : public UIComponent {
  private:
    sf::FloatRect m_bounds;

  public:
    AreaComponent(
      std::string id,
      sf::FloatRect bounds,
      std::function<void(MenuData&)> function,
      bool permanent = true
    )
      : UIComponent(id),
      m_bounds(bounds)
    {
      new_click_event(click_events, id, function, m_bounds, sf::Mouse::Button::Left, permanent ? this : nullptr);
    }

    ~AreaComponent() = default;
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
