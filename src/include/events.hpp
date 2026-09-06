#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <SFML/Window/Event.hpp>
#include <limits>
#include "data.hpp"

class UIComponent; // Forward declare

struct UIEvent {
  std::string id;
  sf::FloatRect bounds;
  sf::View view = default_view;
  UIComponent* component = nullptr;
  int rank = 0;
  bool disabled = false;
};

template<typename TEvent, typename TContainer, typename TPredicate, typename THandler, typename THandlerElse>
void on(const sf::Event& event, TContainer& items, TPredicate predicate, THandler handle, THandlerElse handle_else) {
  const auto* e = event.getIf<TEvent>();
  if (!e) return;

  typename TContainer::value_type* best_item = nullptr;
  int max_z_index = std::numeric_limits<int>::min();

  for (auto& item: items) {
    if (item.bounds.contains(window.mapPixelToCoords(e->position, item.view)) && predicate(e, item)) {
      if (item.component) {
        auto rank = std::max(item.component->z_index, item.rank);
        if (!item.component->is_hidden() && (rank > max_z_index)) {
          max_z_index = rank;
          best_item = &item;
        }
      } else if (!item.disabled && (item.rank > max_z_index)) {
        max_z_index = item.rank;
        best_item = &item;
      }
    }
  }

  if (best_item)
    std::cout << "[INFO] Clicked on '" << best_item->id << "'.\n";

  // handle best item and unfocus all other items
  for (size_t i = 0; i < items.size(); i++) { // This kind of loop is required since the container can be mutated while it is being iterated
    auto& item = items[i];
    if (&item == best_item)
      handle(e, &item);

    else
      handle_else(e, &item);
  }
}

template<typename TEvent, typename TContainer, typename TPredicate, typename THandler>
void on_anywhere(const sf::Event& event, TContainer& items, TPredicate predicate, THandler handle) {
  const auto* e = event.getIf<TEvent>();
  if (!e) return;
  for (size_t i = 0; i < items.size(); i++) { // This type of loop is required for the same reason as in 'void on (at event.hpp:33 (PS: If this number is off it's because I forgot to change it (also I wrote this at 22:42 on 8/8/26) (PPS: You can freely submit a pull request with it corrected and tell me I am lazy)))'
    auto& item = items[i];
    if (item.component) {
      if (!item.component->is_hidden() && predicate(e, item))
        handle(e, &item);
    } else if (!item.disabled && predicate(e, item)) {
      handle(e, &item);
    }
  }
}

template<typename TUIEvent>
bool remove_if_event(std::vector<TUIEvent>& container, std::string id) {
  size_t i = 0;
  for (const auto& uievent : container) {
    if (uievent.id == id) {
      std::cout << "[INFO] Removing event with id='" + id + "'.\n";
      container.erase(container.begin() + i);
      return true;
    }
    i++;
  }

  std::cout << "[INFO] Tried to remove event with id='" + id + "' but it doesn't exist.\n";
  return false;
}

template<typename TUIEvent>
void remove_event(std::vector<TUIEvent>& container, std::string id) {
  if (!remove_if_event(container, id)) {
    throw std::runtime_error("Tried to remove event that doesn't exist (id='" + id + "'')");
  }
}

struct ClickEvent : UIEvent {
  std::function<void()> function;
  sf::Mouse::Button mouse_button;
};

extern std::vector<ClickEvent> click_events;
extern std::vector<ClickEvent> search_res_click_events;
void new_click_event(
  std::vector<ClickEvent>& container,
  std::string id,
  std::function<void()> function,
  sf::FloatRect bounds,
  sf::Mouse::Button mouse_button,
  UIComponent* component = nullptr,
  sf::View view = default_view,
  int rank = 0
);

struct ReleaseEvent : UIEvent {
  std::function<void()> function;
  sf::Mouse::Button mouse_button;
};

extern std::vector<ReleaseEvent> release_events;
extern std::vector<ReleaseEvent> search_res_release_events;
void new_release_event(
  std::vector<ReleaseEvent>& container,
  std::string id,
  std::function<void()> function,
  sf::Mouse::Button mouse_button,
  UIComponent* component = nullptr,
  int rank = 0
);

struct HoverEvent : UIEvent {
  std::function<void()> on_hover_function;
  std::function<void()> off_hover_function;
};

extern std::vector<HoverEvent> hover_events;
void new_hover_event(
  std::vector<ClickEvent>& container,
  std::string id,
  std::function<void()> on_function,
  std::function<void()> off_function,
  sf::FloatRect bounds,
  UIComponent* component,
  sf::View view = default_view,
  int rank = 0
);

struct FocusEvent : UIEvent {
  std::function<void(sf::Vector2f&)> function;
  std::function<void()> else_function;
  sf::Mouse::Button mouse_button;
};

extern std::vector<FocusEvent> focus_events;
void new_focus_event(
  std::vector<FocusEvent>& container,
  std::string id,
  std::function<void(sf::Vector2f&)> function, // Will get called if the click is within bounds
  std::function<void()> else_function, // Will get called if click is out of bounds
  sf::FloatRect bounds,
  sf::Mouse::Button mouse_button,
  UIComponent* component = nullptr,
  sf::View view = default_view,
  int rank = 0
);

struct ScrollEvent : UIEvent {
  float* scroll_offset;
  bool* can_scroll;
};

extern std::vector<ScrollEvent> scroll_events;
void new_scroll_event(
  std::vector<ScrollEvent>& container,
  std::string id,
  sf::FloatRect bounds,
  float* scroll_offset,
  bool* can_scroll,
  UIComponent* component = nullptr,
  int rank = 0
);

class InputComponent; // Forward declare InputComponent so TextEvent can use it

struct TextEvent : UIEvent {
  InputComponent* input_component;
};

extern std::vector<TextEvent> text_events;
void new_text_event(
  std::vector<TextEvent>& container,
  std::string id,
  InputComponent* input_component,
  UIComponent* component = nullptr,
  int rank = 0
);


struct KbEvent : UIEvent {};

extern std::vector<KbEvent> kb_events;
void new_kb_event(
  std::vector<KbEvent>& container,
  std::string id,
  UIComponent* component = nullptr,
  int rank = 0
);

#endif
