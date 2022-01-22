#include <string>

#include "Render.hpp"

constexpr auto WINDOW_WIDTH = 640;
constexpr auto WINDOW_HEIGHT = 480;

constexpr auto TICKS_PER_SEC = 60;
constexpr auto TICKS_PER_NSEC = TICKS_PER_SEC / 1e9;

class Game {
public:
  Game(std::string header, std::string background) : render{WINDOW_WIDTH, WINDOW_HEIGHT, header, background} {};
  void init();
  void run();

private:
  Render render;
  entt::registry registry;

  void update(entt::registry &);
};