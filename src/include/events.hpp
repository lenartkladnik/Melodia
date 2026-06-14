#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <SFML/Window/Event.hpp>
#include <limits>

template<typename TEvent, typename TContainer, typename TPredicate, typename THandler>
void on(const sf::Event& event, TContainer& items, TPredicate predicate, THandler handle) {
  const auto* e = event.getIf<TEvent>();
  if (!e) return;

  const typename TContainer::value_type* best_item = nullptr;
  int max_z_index = std::numeric_limits<int>::min();

  for (const auto& item : items) {
    if (item.bounds.contains(window.mapPixelToCoords(e->position, item.view)) && predicate(e, item)) {
      if (item.component) {
        if (!item.component->hidden && item.component->z_index > max_z_index) {
          max_z_index = item.component->z_index;
          best_item = &item;
        }
      } else {
        // If not component is attached to the event simply default to a z_index of 0
        if (max_z_index < 0) {
          max_z_index = 0;
          best_item = &item;
        }
      }
    }
  }

  if (best_item)
    handle(e, best_item);
}

template<typename TEvent, typename TContainer, typename TPredicate, typename THandler>
  requires (!requires(TEvent e) { e.position; })
void on(const sf::Event& event, TContainer& items, TPredicate predicate, THandler handle) {
  const auto* e = event.getIf<TEvent>();
  if (!e) return;
  for (const auto& item : items) {
    if (item.component) {
      if (!item.component->hidden && predicate(e, item))
        handle(e, &item);
    } else if (predicate(e, item)) {
      handle(e, &item);
    }
  }
}

#endif
