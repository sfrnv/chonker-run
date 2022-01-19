#include <string>

#include "Render.hpp"

constexpr auto TICKS_PER_SEC = 60;
constexpr auto TICKS_PER_NSEC = TICKS_PER_SEC / 1e9;

class Game {
public:
  Game(std::string header, std::string background) : render{header, background} {};
  void run();

private:
  Render render;

  void update(entt::registry &);
};