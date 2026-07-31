#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <SFML/Window/Event.hpp>
#include <limits>

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

#endif
