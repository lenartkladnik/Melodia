#ifndef UTILS_HPP
#define UTILS_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <future>
#include <chrono>

#include "../../external/lib/BS_thread_pool.hpp"

void debug_draw_bounds(sf::RenderTexture& window, sf::FloatRect bounds);
template<typename TIterable>
void debug_print_iterable(TIterable& iterable, std::string sep = ", ") {
  std::cout << "{";
  for (const auto& i : iterable) {
    std::cout << i << sep;
  }
  std::cout << "\x1b[" + std::to_string(sep.size()) + "D";
  std::cout << "}";
}


void set_window(sf::State state);
void draw_window(sf::RenderWindow& render_window, sf::RenderTexture& window);
void new_random();
std::string exec(const char* cmd);
bool resize_image(std::string path, std::string output, sf::Vector2u target_size);
bool rasterize_texture(std::string name);
void rasterize_textures();
std::shared_ptr<sf::Texture> load_texture(std::string name, bool no_invert = false);
bool is_string_valid_name(const std::string& s);
void mkdir(std::string path);
bool must_exist(std::string path);
std::string get_stem(std::string path);
std::string insert_in_stem(std::string path, std::string s);
std::string get_next_available_path(std::string path);
sf::Vector2f find_character_pos(const sf::Text& text, size_t index);
sf::Vector2f find_character_size(const sf::Text& text, size_t index);
size_t find_character_at_pos_x(const std::u32string& string, const sf::Text& text, float pos_x);
inline sf::Vector2f get_mouse_pos(sf::RenderWindow& window) { return window.mapPixelToCoords(sf::Mouse::getPosition(window)); }
std::string u32_to_utf8(const std::u32string& u32);
std::u32string utf8_to_u32(const std::string& utf8);
float matching_song(std::u32string query, std::u32string song_title, std::u32string song_artist, size_t threshold);
std::string seconds_to_human_readable(float seconds);
float getFontOffsetPixels(float target_size);
void setFontSize(sf::Text& text, float target_size, unsigned int raster_mul = 2);
void reset_globals();
sf::Color sub_colors(sf::Color a, sf::Color b);
sf::Color add_colors(sf::Color a, sf::Color b);
sf::Color add_int_to_color(sf::Color a, int b);
float dot_colors(sf::Color a, float wr, float wg, float wb);
sf::Color adjust_if_dark_mode(sf::Color a);
bool color_less_than_color(sf::Color a, sf::Color b);
bool is_color_black(sf::Color a);
std::string escape_csv(const std::string& s);
void start_drag_and_drop();
bool was_unintentional_drag_and_drop();
std::string title_string(const std::string& s);
void cleanup();

template<typename TChar>
std::vector<std::basic_string<TChar>> split_string(const std::basic_string<TChar>& s, TChar delim) {
  std::vector<std::basic_string<TChar>> result;
  if (s.empty()) return result;
  size_t from = 0;
  while (1) {
    size_t pos = s.find(delim, from);
    if (pos == std::string::npos) {
      if (from < s.size()) {
        result.emplace_back(s.substr(from));
      }
      break;
    }
    auto subs = s.substr(from, pos - from);
    if (!subs.empty())
      result.emplace_back(subs);
    from = pos + 1;
  }
  return result;
}

template<typename T>
bool is_future_ready(const std::future<T>& f) {
  return f.valid() && f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

template<typename TShape>
void setGlobalBounds(TShape& target, const sf::FloatRect refBounds) {
  sf::FloatRect localBounds = target.getLocalBounds();

  if (localBounds.size.x == 0.f || localBounds.size.y == 0.f) // Would be division by 0
    return;

  float scaleX = refBounds.size.x / localBounds.size.x;
  float scaleY = refBounds.size.y / localBounds.size.y;
  target.setScale(sf::Vector2f{scaleX, scaleY});

  target.setPosition(sf::Vector2f{
    refBounds.position.x - localBounds.position.x * scaleX,
    refBounds.position.y - localBounds.position.y * scaleY
  });
}

inline BS::thread_pool<> multistate_future_pool;

template<typename T>
class MultistateFuture {
  public:
    std::vector<std::future<T>> futures;
    std::vector<size_t> desires;

    template<typename TFunc>
    void launch(TFunc func, size_t desire) {
      futures.emplace_back(multistate_future_pool.submit_task(std::move(func)));
      desires.push_back(desire);
    }

    std::optional<T> get() {
      size_t i = 0;
      int max_desire = -1;

      std::optional<T> result;

      for (auto& future : futures) {
        if (is_future_ready(future)) {
          if ((int)desires[i] > max_desire) {
            max_desire = (int)desires[i];
            result = future.get();
          }
        }

        i++;
      }

      return result;
    }

    void wait_for(size_t min_desire) {
      while (1) {
        bool all_ready = true;

        size_t i = 0;
        for (const auto& future : futures) {
          if (is_future_ready(future)) {
            if (desires[i] >= min_desire)
              break;
          } else {
            all_ready = false;
          }

          i++;
        }

        if (all_ready) // Even if min_desire wasn't hit exit when all the futures complete
          break;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    }

    void wait() {
      while (1) {
        bool all_ready = true;

        for (const auto& future : futures) {
          if (!is_future_ready(future))
            all_ready = false;
        }

        if (all_ready)
          break;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    }
};

#endif
