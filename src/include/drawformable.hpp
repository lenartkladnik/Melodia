#ifndef DRAWFORMABLE_HPP
#define DRAWFORMABLE_HPP

// Adapted from:
// https://en.sfml-dev.org/forums/index.php?topic=24133.0

class DrawformableObject {
 public:
  DrawformableObject(std::shared_ptr<sf::Drawable> drawable, std::shared_ptr<sf::Transformable> transformable) {
    drawable_ = drawable;
    transformable_ = transformable;
  }

  sf::Drawable& get_drawable() {
    return *drawable_;
  }

  sf::Transformable& get_transformable() {
    return *transformable_;
  }

  // Unsafe getGlobalBounds implementation, requires drawable_ to be a sf::Transformable object
  sf::FloatRect getGlobalBounds() const {
    // Try Sprite
    if (auto sprite = dynamic_cast<sf::Sprite*>(drawable_.get())) {
        return transformable_->getTransform().transformRect(sprite->getLocalBounds());
    }

    // Try Shape (CircleShape, RectangleShape, etc.)
    if (auto shape = dynamic_cast<sf::Shape*>(drawable_.get())) {
        return transformable_->getTransform().transformRect(shape->getLocalBounds());
    }

    // Try Text
    if (auto text = dynamic_cast<sf::Text*>(drawable_.get())) {
        return transformable_->getTransform().transformRect(text->getLocalBounds());
    }

    throw std::runtime_error("getGlobalBounds() not supported for this drawable type");
  }

private:
  std::shared_ptr<sf::Drawable> drawable_;
  std::shared_ptr<sf::Transformable> transformable_;
};

// ====================================================================

struct DTPair {
  std::shared_ptr<DrawformableObject> drawformable;
  std::shared_ptr<sf::Texture> texture;
};

struct DTCache {
  std::vector<int> ids;
  std::vector<std::vector<std::string>> names;
  std::vector<std::vector<DTPair>> items;

  size_t find(int id) const {
    return std::distance(ids.begin(), std::find(ids.begin(), ids.end(), id));
  }

  int name_to_index(int id, const std::string& name) const {
    auto idx = this->find(id);
    auto& names_vec = names[idx];

    return std::distance(names_vec.begin(), std::find(names_vec.begin(), names_vec.end(), name));
  }

  bool contains(int id) const {
    return (ids.begin() + this->find(id)) != ids.end();
  }

  void add(int id, std::string name, const DTPair& dt) {
    if (this->contains(id)) {
      auto idx = this->find(id);
      auto& vec = items[idx];
      vec.push_back(dt);
      names[idx].push_back(name);
    }
    else {
      ids.push_back(id);
      items.push_back({dt});
      names.push_back({name});
    }
  }

  void remove(int id) {
    auto idx = this->find(id);

    ids.erase(ids.begin() + idx);
    names.erase(names.begin() + idx);
    items.erase(items.begin() + idx);
  }

  void clear() {
    ids.clear();
    names.clear();
    items.clear();
  }

  void draw(int id, sf::RenderWindow& window) const {
    auto& vec = items[this->find(id)];

    for (auto dt : vec) {
      window.draw(dt.drawformable->get_drawable());
    }
  }

  const DTPair& get_item(int id, int index) const {
    return items[this->find(id)][index];
  }
};

#endif
