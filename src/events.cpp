#include "include/events.hpp"

std::vector<ClickEvent> click_events;
std::vector<ClickEvent> search_res_click_events;

std::vector<ReleaseEvent> release_events;
std::vector<ReleaseEvent> search_res_release_events;

std::vector<HoverEvent> hover_events;

std::vector<FocusEvent> focus_events;

std::vector<ScrollEvent> scroll_events;

std::vector<TextEvent> text_events;

std::vector<KbEvent> kb_events;

template<typename T>
void _handle_duplicates(T& v, std::string& id) {
  // A call with the same id should 'update' an existing entry

  for (auto it = v.begin(); it != v.end(); it++) {
    if (it->id == id) {
      v.erase(it); // Invalidates it so a break is mandatory
      break;
    }
  }
}

void new_click_event(std::vector<ClickEvent>& container, std::string id, std::function<void()> function, sf::FloatRect bounds, sf::Mouse::Button mouse_button, UIComponent* component, sf::View view, int rank) {
  _handle_duplicates(container, id);
  container.push_back(ClickEvent{{std::move(id), bounds, view, component, rank}, function, mouse_button});
}

void new_release_event(std::vector<ReleaseEvent>& container, std::string id, std::function<void()> function, sf::Mouse::Button mouse_button, UIComponent* component, int rank) {
  _handle_duplicates(container, id);
  container.push_back(ReleaseEvent{{std::move(id), {}, {}, component, rank}, function, mouse_button});
}

void new_hover_event(std::vector<HoverEvent>& container, std::string id, std::function<void()> on_function, std::function<void()> off_function, sf::FloatRect bounds, UIComponent* component, sf::View view, int rank) {
  _handle_duplicates(container, id);
  container.push_back(HoverEvent{{std::move(id), bounds, view, component, rank}, on_function, off_function});
}

void new_focus_event(std::vector<FocusEvent>& container, std::string id, std::function<void(sf::Vector2f&)> function, std::function<void()> else_function, sf::FloatRect bounds, sf::Mouse::Button mouse_button, UIComponent* component, sf::View view, int rank) {
  _handle_duplicates(container, id);
  container.push_back(FocusEvent{{std::move(id), bounds, view, component, rank}, function, else_function, mouse_button});
}

void new_scroll_event(std::vector<ScrollEvent>& container, std::string id, sf::FloatRect bounds, float* scroll_offset, bool* can_scroll, UIComponent* component, int rank) {
  _handle_duplicates(container, id);
  container.push_back(ScrollEvent{{std::move(id), bounds, default_view, component, rank}, scroll_offset, can_scroll});
}

void new_text_event(std::vector<TextEvent>& container, std::string id, InputComponent* input_component, UIComponent* component, int rank) {
  _handle_duplicates(container, id);
  container.push_back(TextEvent{{std::move(id), {}, {}, component, rank}, input_component});
}

void new_kb_event(std::vector<KbEvent>& container, std::string id, UIComponent* component, int rank) {
  _handle_duplicates(container, id);
  container.push_back(KbEvent{{std::move(id), {}, {}, component, rank}});
}

