#include "include/data.hpp"
#include "include/song_containers.hpp"
#include "../external/lib/RoundedRectangleShape.hpp"

std::shared_ptr<SmallSongContainerComponent> create_small_song_container(int song_id, sf::Vector2f position, sf::Vector2f size) {
  std::string song_path = base_music_path_data + std::to_string(song_id);

  auto cover_texture = std::make_shared<sf::Texture>(song_path + ".small.png");
  sf::RoundedRectangleShape cover({queue_cover_size, queue_cover_size}, 8, main_n);
  cover.setTexture(cover_texture.get());
  cover.setPosition(position);

  sf::RoundedRectangleShape cover_shadow({queue_cover_size, queue_cover_size}, 8, main_n);
  cover_shadow.setFillColor(dark_background_shadow_color);
  cover_shadow.setPosition({cover.getPosition().x + small_shadow_offset, cover.getPosition().y + small_shadow_offset});

  auto title = std::make_shared<sf::Text>(default_font, get_song_title(song_id));
  title->setFillColor(title_color);
  setFontSize(*title, small_font_size);
  title->setPosition({
    cover.getPosition().x + cover.getGlobalBounds().size.x + 15.f,
    cover.getPosition().y + queue_cover_size / 3 - 10.f
  });

  auto artist = std::make_shared<sf::Text>(default_font, get_song_artist(song_id));
  artist->setFillColor(artist_color);
  setFontSize(*artist, small_font_size);
  artist->setPosition({title->getPosition().x, title->getPosition().y + 20.f});

  sf::RoundedRectangleShape background(size, 8, main_n);
  background.setFillColor(background_shadow_color);
  background.setPosition({cover.getPosition().x - 5.f, cover.getPosition().y - 5.f});

  auto more = std::make_shared<sf::Text>(default_font, "...");
  more->setFillColor(text_color);
  setFontSize(*more, small_font_size);
  more->setPosition({
    background.getPosition().x + background.getGlobalBounds().size.x - more->getGlobalBounds().size.x - 20.f,
    (background.getPosition().y - 5.f) + (background.getGlobalBounds().size.y - 5.f) / 2 - more->getGlobalBounds().size.y
  });

  auto small_song_container_component = std::make_shared<SmallSongContainerComponent>();
  small_song_container_component->background = background;
  small_song_container_component->cover_shadow = cover_shadow;
  small_song_container_component->cover = cover;
  small_song_container_component->title = title;
  small_song_container_component->artist = artist;
  small_song_container_component->more = more;
  small_song_container_component->cover_tex = cover_texture;
  return small_song_container_component;
}

void draw_small_song_container(std::shared_ptr<SmallSongContainerComponent> small_song_container) {
  window.draw(small_song_container->background);
  window.draw(small_song_container->cover_shadow);
  window.draw(small_song_container->cover);
  window.draw(*small_song_container->title.value());
  window.draw(*small_song_container->artist.value());
  window.draw(*small_song_container->more.value());
}
