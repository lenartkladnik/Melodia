#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <SFML/Window/Event.hpp>
#include <limits>
#include "data.hpp"

class UIComponent; // Forward declare

template<typename TEvent, typename TContainer, typename TPredicate, typename THandler, typename THandlerElse>
void on(const sf::Event& event, TContainer& items, TPredicate predicate, THandler handle, THandlerElse handle_else) {
  const auto* e = event.getIf<TEvent>();
  if (!e) return;

  typename TContainer::value_type* best_item = nullptr;
  int max_z_index = std::numeric_limits<int>::min();

  for (auto& item : items) {
    if (item.bounds.contains(window.mapPixelToCoords(e->position, item.view)) && predicate(e, item)) {
      if (item.component) {
        if (!item.component->is_hidden() && (item.component->z_index > max_z_index)) {
          max_z_index = item.component->z_index;
          best_item = &item;
        }
      } else if (!item.disabled && (item.rank > max_z_index)) {
        max_z_index = item.rank;
        best_item = &item;
      }
    }
  }

  // handle best item and unfocus all other items
  for (auto& item : items) {
    if (&item == best_item)
      handle(e, best_item);

    else
      handle_else(e, &item);
  }
}

template<typename TEvent, typename TContainer, typename TPredicate, typename THandler>
void on_anywhere(const sf::Event& event, TContainer& items, TPredicate predicate, THandler handle) {
  const auto* e = event.getIf<TEvent>();
  if (!e) return;
  for (auto& item : items) {
    if (item.component) {
      if (!item.component->is_hidden() && predicate(e, item))
        handle(e, &item);
    } else if (!item.disabled && predicate(e, item)) {
      handle(e, &item);
    }
  }
}

struct UIEvent {
  std::string id;
  sf::FloatRect bounds;
  sf::View view = default_view;
  UIComponent* component = nullptr;
  int rank = 0;
  bool disabled = false;
};

template<typename TUIEvent>
bool remove_if_event(std::vector<TUIEvent>& container, std::string id) {
  size_t i = 0;
  for (const auto& uievent : container) {
    if (uievent.id == id) {
      container.erase(container.begin() + i);
      return true;
    }
    i++;
  }

  return false;
}

template<typename TUIEvent>
void remove_event(std::vector<TUIEvent>& container, std::string id) {
  if (!remove_if_event(container, id)) {
    throw "Error: Tried to remove event that doesn't exist (id='" + id + "'')";
  }
}

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
  sf::View view = default_view,
  int rank = 0
);

struct ReleaseEvent : UIEvent {
  std::function<void(MenuData&)> function;
  sf::Mouse::Button mouse_button;
};

extern std::vector<ReleaseEvent> release_events;
extern std::vector<ReleaseEvent> search_res_release_events;
void new_release_event(
  std::vector<ReleaseEvent>& container,
  std::string id,
  std::function<void(MenuData&)> function,
  sf::Mouse::Button mouse_button,
  UIComponent* component = nullptr,
  int rank = 0
);

struct HoverEvent : UIEvent {
  std::function<void(MenuData&)> on_hover_function;
  std::function<void(MenuData&)> off_hover_function;
};

extern std::vector<HoverEvent> hover_events;
void new_hover_event(
  std::vector<ClickEvent>& container,
  std::string id,
  std::function<void(MenuData&)> on_function,
  std::function<void(MenuData&)> off_function,
  sf::FloatRect bounds,
  UIComponent* component,
  sf::View view = default_view,
  int rank = 0
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
  sf::View view = default_view,
  int rank = 0
);

struct ScrollEvent : UIEvent {
  float scroll_offset;
  bool can_scroll;
};

extern std::vector<ScrollEvent> scroll_events;
void new_scroll_event(
  std::vector<ScrollEvent>& container,
  std::string id,
  sf::FloatRect bounds,
  float& scroll_offset,
  bool& can_scroll,
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
