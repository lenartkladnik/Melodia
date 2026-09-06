#include "include/data.hpp"
#include "include/song_containers.hpp"
#include "include/storage_handler.hpp"

#include "../external/lib/RoundedRectangleShape.hpp"

std::shared_ptr<SmallSongContainerComponent> create_small_song_container(int song_id, sf::Vector2f position, sf::Vector2f size, bool dragging) {
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
  background.setFillColor(sf::Color{
      background_shadow_color.r,
      background_shadow_color.g,
      background_shadow_color.b,
      (uint8_t)(dragging ? 90 : background_shadow_color.a) // Make the container slightly transparent when dragging
  });
  background.setPosition({cover.getPosition().x - 5.f, cover.getPosition().y - 5.f});

  auto small_song_container_component = std::make_shared<SmallSongContainerComponent>();
  small_song_container_component->background = std::move(background);
  small_song_container_component->cover_shadow = std::move(cover_shadow);
  small_song_container_component->cover = std::move(cover);
  small_song_container_component->title = std::move(title);
  small_song_container_component->artist = std::move(artist);
  small_song_container_component->cover_tex = std::move(cover_texture);
  return small_song_container_component;
}

void draw_small_song_container(std::shared_ptr<SmallSongContainerComponent> small_song_container) {
  window.draw(small_song_container->background);
  window.draw(small_song_container->cover_shadow);
  window.draw(small_song_container->cover);
  window.draw(*small_song_container->title.value());
  window.draw(*small_song_container->artist.value());
}
