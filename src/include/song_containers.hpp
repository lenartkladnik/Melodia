#ifndef SONG_CONTAINERS_HPP
#define SONG_CONTAINERS_HPP

#include "data.hpp"
#include "../../external/lib/RoundedRectangleShape.hpp"

struct SmallSongContainerComponent {
  sf::RoundedRectangleShape background;
  sf::RoundedRectangleShape cover_shadow;
  sf::RoundedRectangleShape cover;
  std::optional<std::shared_ptr<sf::Text>> title;
  std::optional<std::shared_ptr<sf::Text>> artist;
  std::optional<std::shared_ptr<sf::Text>> more;
  std::shared_ptr<sf::Texture> cover_tex;
};

std::shared_ptr<SmallSongContainerComponent> create_small_song_container(int song_id, sf::Vector2f position, sf::Vector2f size);
void draw_small_song_container(std::shared_ptr<SmallSongContainerComponent> small_song_container);

#endif
