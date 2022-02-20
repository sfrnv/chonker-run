#include <string>

#ifndef RENDER_H
#define RENDER_H

#include "Render.hpp"

#endif
#include "World.hpp"

constexpr auto WINDOW_WIDTH = 640;
constexpr auto WINDOW_HEIGHT = 480;

constexpr auto TICKS_PER_SEC = 60;
constexpr auto TICKS_PER_NSEC = TICKS_PER_SEC / 1e9;

class Game {
public:
  Game(const std::string &title, const std::string &sprite_path,
       const std::initializer_list<std::string> level_layers)
      : render{WINDOW_WIDTH, WINDOW_HEIGHT, title, sprite_path},
        world{level_layers} {};
  void run();

private:
  Render render;
  World world;
};