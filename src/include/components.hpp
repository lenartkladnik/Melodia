#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include <string>
#include <memory>
#include <SFML/Window/Export.hpp>

#include "data.hpp"
#include "events.hpp"
#include "signals.hpp"

class PopupComponent;
extern std::unordered_map<std::string, std::shared_ptr<PopupComponent>> popup_components;

class UIComponent {
  public:
    std::string id;
    int z_index;
    bool m_hidden = false;
    bool m_focused = false;
    bool m_hover = false;

    virtual void draw() {};

    virtual void focus(const sf::Vector2f&) {
      m_focused = true;
    }

    virtual void unfocus() {
      m_focused = false;
    }

    bool is_focused() {
      return m_focused;
    }

    virtual void hide() {
      m_hidden = true;
    }

    virtual void show() {
      m_hidden = false;
    }

    bool is_hidden() {
      return m_hidden;
    }

    virtual void on_hover() {
      m_hover = true;
    }

    virtual void off_hover() {
      m_hover = false;
    }

    void set_z_index(int new_z_index) {
      z_index = new_z_index;
    }

    UIComponent(std::string id, bool hidden = false)
      : id(std::move(id))
    {
      z_index = global_z_index;
      global_z_index++;

      if (hidden)
        hide();
    }

    virtual ~UIComponent() = default;
};

class InputComponent : public UIComponent {
  private:
    std::string component_id;
    std::u32string input_string = U"";
    size_t cursor_pos = 0;
    bool show_cursor = false;
    bool reset_cursor_flag = false;
    bool input_active = false;
    bool refresh_input_flag = true;
    sf::Clock cursor_clock;
    std::u32string prev_input_string = U"";
    std::u32string input_prompt;
    float corner_radius;
    sf::RoundedRectangleShape input_background;
    sf::RoundedRectangleShape input_shadow;
    sf::Text input_text;
    std::shared_ptr<sf::Texture> action_button_tex;
    std::optional<sf::Sprite> action_button;
    std::function<void(MenuData&)> action_function;
    size_t selection_start = std::string::npos;
    size_t selection_end = std::string::npos;
    bool selecting = false;
    sf::RoundedRectangleShape selection_background;
    std::u32string selected_text;
    bool text_replica = false;
    bool select_all_on_click = false;
    bool focus_event_else_is_empty = false;
    size_t normal_s_start = std::string::npos; // normal_s_start is guaranteed to be <= normal_s_end
    size_t normal_s_end = std::string::npos;

  public:
    // Arguments for constructors
    struct Args {
      struct InputField {
        std::string id;
        sf::Vector2f size;
        sf::Vector2f pos;
        std::u32string prompt = U"";
        std::shared_ptr<sf::Texture> action_tex = nullptr;
        std::function<void(MenuData&)> action_function = [](MenuData&){};
        float corner_radius = 20;
        bool hidden = false;
        bool no_focus_event_else = false;
      };

      struct TextReplica {
        std::string id;
        std::shared_ptr<sf::Text> text_reference;
        float font_size;
        bool select_all_on_click = false;
      };
    };

    // Input field input component
    InputComponent(MenuData& menu_data, Args::InputField arguments)
      : UIComponent(arguments.id, arguments.hidden),
        input_prompt(arguments.prompt),
        corner_radius(arguments.corner_radius),
        input_text(default_font, arguments.prompt, 20),
        action_function(arguments.action_function),
        focus_event_else_is_empty(arguments.no_focus_event_else)

    {
      component_id = arguments.id;

      if (arguments.action_tex) {
        action_button_tex = arguments.action_tex;
        action_button.emplace(*action_button_tex);
      }

      input_string = U"";
      cursor_pos = 0;

      input_background.setSize(arguments.size);
      input_background.setCornerPointCount(main_n);
      input_background.setPosition(arguments.pos);
      input_background.setFillColor(light_background_color);
      background_reset_corner_radii();

      input_shadow.setSize({input_background.getGlobalBounds().size.x + 5.f, input_background.getGlobalBounds().size.y + 5.f});
      input_shadow.setCornersRadius(arguments.corner_radius);
      input_shadow.setCornerPointCount(main_n);
      input_shadow.setPosition({input_background.getPosition().x - 2.5f, input_background.getPosition().y + 3.f});
      input_shadow.setFillColor(dark_main_color);

      input_text.setFillColor(light_text_color);
      setFontSize(input_text, medium_font_size);
      input_text.setPosition({
        input_background.getPosition().x + 16.f,
        input_background.getPosition().y + input_text.getGlobalBounds().size.y - 2.5f
      });

      if (action_button) {
        action_button->setPosition({
          input_background.getPosition().x + input_background.getGlobalBounds().size.x - action_button->getGlobalBounds().size.x - 12.f,
          input_background.getPosition().y + 5.f
        });

        register_action(arguments.action_function);
      }

      selection_background.setFillColor(selection_color);
      selection_background.setCornerPointCount(main_n);
      selection_background.setCornersRadius(4);

      new_focus_event(focus_events, id,
        [this](MenuData&, sf::Vector2f pos) {
          if (!is_hidden())
            focus(pos);
        },
        [this](MenuData&) {
          if (!this->focus_event_else_is_empty) {
            unfocus();
          }
        },
      input_background.getGlobalBounds(), sf::Mouse::Button::Left, this);

      new_text_event(text_events, id, this, this);

      new_release_event(release_events, id,
        [this](MenuData&) {
          selecting = false;
        },
        sf::Mouse::Button::Left, this);

      connect_signals(menu_data);
    }

    // Text replica input component
    InputComponent(MenuData& menu_data, Args::TextReplica arguments)
      : UIComponent(arguments.id),
        input_prompt(arguments.text_reference->getString().toUtf32()),
        input_text(default_font, arguments.text_reference->getString(), 20),
        select_all_on_click(arguments.select_all_on_click)
    {
      component_id = arguments.id;
      text_replica = true;

      input_string = arguments.text_reference->getString().toUtf32();
      cursor_pos = 0;

      input_background.setPosition({arguments.text_reference->getPosition().x, arguments.text_reference->getPosition().y - 5.f});
      input_background.setCornerPointCount(main_n);
      input_background.setSize({arguments.text_reference->getGlobalBounds().size.x, arguments.text_reference->getGlobalBounds().size.y + 10.f});
      update_input_component_events();

      input_text.setFillColor(text_color);
      setFontSize(input_text, arguments.font_size);
      input_text.setPosition(arguments.text_reference->getPosition());

      selection_background.setFillColor(selection_color);
      selection_background.setCornerPointCount(main_n);
      selection_background.setCornersRadius(4);

      new_text_event(text_events, id, this, this);

      new_release_event(release_events, id,
        [this](MenuData&) {
          selecting = false;
        },
      sf::Mouse::Button::Left, this);

      connect_signals(menu_data);
    }

    InputComponent() = delete;

    ~InputComponent() {
      remove_if_event(text_events, component_id);
      remove_if_event(release_events, component_id);
      remove_if_event(click_events, component_id + "_action_button");
      remove_if_event(focus_events, component_id);
      disconnect_signals();
    }

    // Disallow copy
    InputComponent(const InputComponent&) = delete;
    InputComponent& operator=(const InputComponent&) = delete;

    // Disallow move
    InputComponent(InputComponent&&) = delete;
    InputComponent& operator=(InputComponent&&) = delete;

    void connect_signals(MenuData& menu_data) {
      MenuData* menu_data_ptr = &menu_data;

      copy_signal.connect(component_id, [this](){ if (this->m_focused) this->copy(); });
      paste_signal.connect(component_id, [this](){ if (this->m_focused) this->paste(); });
      select_all_signal.connect(component_id, [this](){ if (this->m_focused) this->select_all(); });
      left_signal.connect(component_id, [this](){ if (this->m_focused) this->move_cursor(-1); });
      right_signal.connect(component_id, [this](){ if (this->m_focused) this->move_cursor(1); });
      confirm_signal.connect(component_id, [this, menu_data_ptr](){ if (this->m_focused && this->action_function) this->action_function(*menu_data_ptr); });
      escape_signal.connect(component_id, [this](){ if (this->m_focused) this->unfocus(); });
    }

    void disconnect_signals() {
      copy_signal.disconnect(component_id);
      paste_signal.disconnect(component_id);
      select_all_signal.disconnect(component_id);
      left_signal.disconnect(component_id);
      right_signal.disconnect(component_id);
      confirm_signal.disconnect(component_id);
      escape_signal.disconnect(component_id);
    }

    void draw() {
      if (!is_hidden()) {
        if (text_replica) {
          if (selection_start != std::string::npos) {
            if (selecting)
              select();
            draw_selection();
          }
          window.draw(input_text);
          if (show_cursor && selected_text.empty())
            draw_cursor();
        } else {
          window.draw(input_background);
          if (selection_start != std::string::npos) {
            if (selecting)
              select();
            draw_selection();
          }
          window.draw(input_text);
          if (action_button)
            window.draw(*action_button);
          if (show_cursor && selected_text.empty())
            draw_cursor();
        }
      }
    }

    void update_input_component_events() {
      if (text_replica) {
        input_background.setSize({input_text.getGlobalBounds().size.x, input_background.getGlobalBounds().size.y});
      }

      remove_if_event(focus_events, component_id);

      new_focus_event(focus_events, id,
        [this](MenuData&, sf::Vector2f pos) {
          if (!is_hidden())
            focus(pos);
        },
        [this](MenuData&) {
          if (!this->focus_event_else_is_empty) {
            unfocus();
          }
        },
      input_background.getGlobalBounds(), sf::Mouse::Button::Left, this);
    }

    void reset_cursor() {
      cursor_clock.restart();
      show_cursor = true;
    }

    void move_cursor(int move) {
      // Keep cursor solid while changing position
      reset_cursor();

      int new_cursor_pos = (int)cursor_pos + move;

      if (new_cursor_pos >= 0 && new_cursor_pos <= (int)input_string.size()) {
        cursor_pos = (size_t)new_cursor_pos;
      }
    }

    void update() {
      if (cursor_clock.getElapsedTime() >= sf::milliseconds(500)) {
        cursor_clock.restart();
        show_cursor = !show_cursor;
      }

      input_text.setString(input_string);
    }

    void delete_selection() {
      if (selected_text.empty()) return;

      input_string.erase(normal_s_start, normal_s_end - normal_s_start); // Remove what was in the selection
      cursor_pos = std::min(normal_s_start, input_string.size());
      deselect();
    }

    void write_char(char32_t input, bool unblock = false) {
      if (!m_focused && !unblock) return; // Refuse to write

      // Keep cursor solid while inputting
      reset_cursor();

      if (input < 32 || input == 127) {
        switch (input) {
          case 8: // Backspace
            if (!selected_text.empty()) {
              delete_selection();
            } else if (input_string.size() > 0 && cursor_pos > 0) {
              cursor_pos--;
              input_string.erase(cursor_pos, 1);
            }
            break;

          case 127: // Delete
            if (!selected_text.empty()) {
              delete_selection();
            } else if (input_string.size() > 0 && cursor_pos < input_string.size()) {
              input_string.erase(cursor_pos, 1);
            }

            break;
        }
      }
      else {
        std::u32string str_input;
        str_input += input;

        if (!selected_text.empty()) {
          input_string.erase(normal_s_start, normal_s_end - normal_s_start); // Remove what was in the selection
          input_string.insert(normal_s_start, str_input);
          cursor_pos = std::min(normal_s_start + 1, input_string.size());
          deselect();
        } else {
          input_string.insert(cursor_pos, str_input);
          cursor_pos++;
        }

      }

      update();
      update_input_component_events();
    }

    void write(std::u32string input, bool unblock = false) {
      for (const char32_t ch : input) {
        write_char(ch, unblock);
      }
    }

    void copy() {
      if (m_focused && !selected_text.empty()) {
        sf::Clipboard::setString(selected_text);
      }
    }

    void paste() {
      if (m_focused) {
        write(sf::Clipboard::getString().toUtf32());
      }
    }

    void select_all() {
      if (m_focused) {
        selection_start = 0;
        selection_end = input_string.size();
        selected_text = input_string;
        normal_s_start = selection_start;
        normal_s_end = selection_end;
      }
    }

    void focus(const sf::Vector2f& pos) override {
      m_focused = true;

      deselect();

      input_active = true;
      input_text.setFillColor(text_color);

      if (pos == sf::Vector2f{-1, -1}) return; // Prevent any extra actions (selecting, changing cursor position, ...)

      selecting = true;

      if (input_string.empty()) {
        cursor_pos = 0;
      }
      else {
        cursor_pos = find_character_at_pos_x(input_string, input_text, pos.x);
        if (select_all_on_click) {
          select_all();
          selecting = false; // Don't drag selectiomn
        } else {
          selection_start = cursor_pos;
        }

        // Keep cursor solid while changing cursor pos
        reset_cursor();
      }
    }

    void unfocus() {
      m_focused = false;

      deselect();

      input_active = false;
      show_cursor = false;

      if (input_string.size() == 0) {
        if (!text_replica) {
          input_text.setFillColor(light_text_color);
        } else {
          input_string = input_prompt; // text lookalikes treat deleting the whole string as "canceling", so the string is set to the default
        }
        input_text.setString(input_prompt);
        update_input_component_events();
      }
    }

    void deselect() {
      selection_start = std::string::npos;
      selection_end = std::string::npos;
      selected_text = U"";
    }

    void select() {
      if (selection_start != std::string::npos) {
        selection_end = find_character_at_pos_x(input_string, input_text, get_mouse_pos(render_window).x);

        normal_s_end = selection_end;
        normal_s_start = selection_start;

        if (normal_s_end < normal_s_start)
          std::swap(normal_s_end, normal_s_start);

        selected_text = input_string.substr(normal_s_start, normal_s_end - normal_s_start);
      }
    }

    void draw_selection() {
      if (selection_start == selection_end)
        return; // No selection

      auto start = find_character_pos(input_text, selection_start);
      auto end = find_character_pos(input_text, selection_end);

      float margin = 4.f;

      selection_background.setPosition({start.x, start.y + margin});
      selection_background.setSize({end.x - start.x, input_text.getGlobalBounds().size.y + (margin * 2)});
      selection_background.move({0.f, -selection_background.getGlobalBounds().size.y});

      window.draw(selection_background);
    }

    bool is_text_too_long() {
      float action_button_size_x = 0.f;

      if (action_button.has_value()) {
        action_button_size_x = action_button.value().getGlobalBounds().size.x + 10.f; // + 10 so the text and action_button aren't touching
      }

      return
        input_text.getGlobalBounds().size.x
        >
        input_background.getGlobalBounds().size.x -
        (input_text.getPosition().x - input_background.getPosition().x) - // Subtract the offset of the text at the start
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
      input_string = U"";
      cursor_pos = 0;
    }

    std::u32string get_input_string() {
      return input_string;
    }

    void draw_cursor() {
      sf::RectangleShape cursor({1.2, input_background.getGlobalBounds().size.y - 10.f});
      cursor.setPosition({find_character_pos(input_text, cursor_pos).x, input_background.getPosition().y + 5.f});
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
};

class ButtonComponent : public UIComponent {
  // TODO:
  // - Add pressed down styling

  private:
    sf::RenderTexture& window;
    sf::RoundedRectangleShape button_shape;
    sf::Text button_text;
    std::function<void(MenuData&)> function;
    sf::Color m_button_shape_color;

  public:
    struct Args {
      struct BasicButton {
        sf::RenderTexture& window;
        std::string id;
        std::string text;
        sf::Vector2f size;
        sf::Vector2f pos;
        std::function<void(MenuData&)> function;
        bool hidden = false;
        sf::Color button_shape_color = dark_background_color;
        sf::Color button_text_color = text_color;
        int corner_radius = 10;
      };
    };

    ButtonComponent(Args::BasicButton arguments)
      : UIComponent(arguments.id, arguments.hidden),
        window(arguments.window),
        button_text(default_font, arguments.text, 0),
        function(arguments.function),
        m_button_shape_color(arguments.button_shape_color)
    {
      button_shape.setSize(arguments.size);
      button_shape.setPosition(arguments.pos);
      button_shape.setFillColor(arguments.button_shape_color);
      button_shape.setCornersRadius(arguments.corner_radius);
      button_shape.setCornerPointCount(main_n);

      int text_size = 0;
      if (arguments.text.size() != 0)
        text_size = (int)((arguments.size.x/arguments.text.size()) * font_multiplier);

      setFontSize(button_text, text_size);
      button_text.setFillColor(arguments.button_text_color);
      button_text.setPosition({
        button_shape.getPosition().x + button_shape.getGlobalBounds().size.x / 2 - button_text.getGlobalBounds().size.x / 2,
        button_shape.getPosition().y + button_shape.getGlobalBounds().size.y / 2 - button_text.getGlobalBounds().size.y
      });

      new_click_event(click_events, arguments.id, [this](MenuData& menu_data) { if (!is_hidden()) this->function(menu_data); }, button_shape.getGlobalBounds(), sf::Mouse::Button::Left, this);
    }

    ~ButtonComponent() = default;

    // Disallow copy
    ButtonComponent(const ButtonComponent&) = delete;
    ButtonComponent& operator=(const ButtonComponent&) = delete;

    // Disallow move
    ButtonComponent(ButtonComponent&&) = delete;
    ButtonComponent& operator=(ButtonComponent&&) = delete;

    void draw() {
      if (!is_hidden()) {
        window.draw(button_shape);
        window.draw(button_text);
      }
    }

    void on_hover() {
      m_hover = true;

      button_shape.setFillColor(add_int_to_color(m_button_shape_color, -hover_sub));
    }

    void off_hover() {
      m_hover = false;

      button_shape.setFillColor(m_button_shape_color);
    }

    sf::FloatRect button_bounds() {
      return button_shape.getGlobalBounds();
    }
};

class PopupComponent : public UIComponent {
  private:
    std::unordered_map<std::string, std::shared_ptr<InputComponent>> input_components;
    std::unordered_map<std::string, std::shared_ptr<ButtonComponent>> button_components;
    std::unordered_map<std::string, sf::RoundedRectangleShape> rounded_rectangle_shapes;

  public:
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

    // Disallow move
    PopupComponent(PopupComponent&&) = delete;
    PopupComponent& operator=(PopupComponent&&) = delete;

    void draw() {
      if (!is_hidden()) {
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
      input_component->z_index += z_index;

      input_components.insert({input_id, input_component});
    }

    void new_button(std::string id, std::shared_ptr<ButtonComponent> button_component) {
      button_component->z_index += z_index;

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

    void hide() {
      m_hidden = true;

      for (auto it = input_components.begin(); it != input_components.end(); it++) {
        it->second->hide();
      }

      for (auto it = button_components.begin(); it != button_components.end(); it++) {
        it->second->hide();
      }
    }

    void show() {
      m_hidden = false;

      for (auto it = input_components.begin(); it != input_components.end(); it++) {
        it->second->show();
      }

      for (auto it = button_components.begin(); it != button_components.end(); it++) {
        it->second->show();
      }
    }
};

class AreaComponent : public UIComponent {
  private:
    sf::FloatRect m_bounds;

  public:
    struct Args {
      struct Area {
        std::string id;
        sf::FloatRect bounds;
        std::function<void(MenuData&)> function;
        sf::View view = default_view;
        bool permanent = true;
        int rank = 0;
      };
    };

    AreaComponent(Args::Area arguments)
      : UIComponent(arguments.id),
      m_bounds(arguments.bounds)
    {
      new_click_event(click_events, arguments.id, arguments.function, m_bounds, sf::Mouse::Button::Left, arguments.permanent ? this : nullptr, arguments.view, arguments.rank);
    }

    ~AreaComponent()
    {
      remove_if_event(click_events, id);
    }

    sf::FloatRect get_bounds() {
      return m_bounds;
    }
};

#endif
