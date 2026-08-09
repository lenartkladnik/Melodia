#ifndef SIGNALS_HPP
#define SIGNALS_HPP

#include <functional>
#include <unordered_map>
#include <string>

class Signal {
  private:
    std::unordered_map<std::string, std::function<void()>> callbacks;

  public:
    void connect(std::string id, std::function<void()> callback) {
      callbacks[id] = std::move(callback);
    }

    void disconnect(std::string id) {
      callbacks.erase(id);
    }

    void reset() {
      callbacks.clear();
    }

    void emit() {
      for (const auto& [_, callback] : callbacks) {
        callback();
      }
    }
};

extern Signal ctrl_c_signal;
extern Signal ctrl_v_signal;
extern Signal ctrl_a_signal;

#endif
