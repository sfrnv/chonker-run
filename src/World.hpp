#include <string>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_rect.h>

#include <entt/entt.hpp>

#ifndef RENDER_H
#define RENDER_H

#include "Render.hpp"

#endif

constexpr auto SCALING_FACTOR = 4;

constexpr auto STONE_PIXEL = 0xFF555555;
constexpr auto GRASS_PIXEL = 0xFF00FF00;
constexpr auto WATER_PIXEL = 0xFFFF0000;
constexpr auto SAND_PIXEL = 0xFF00FFFF;
constexpr auto BRICK_PIXEL = 0xFF0000FF;
constexpr auto LAVA_PIXEL = 0xFF00AAFF;

struct position {
  float x;
  float y;
};

struct velocity {
  float dx;
  float dy;
};

struct focus {
  bool focused;
};

class World {
public:
  int width, height;
  bool updated;

  World(const std::string &path);
  ~World(){};

  void init(std::string level);
  void update(Render &render);

private:
  entt::registry registry;

  void load_tiles(SDL_Surface *image);
  void handle_input();
  void move_entities();
  void focus_camera(Render &render);
  void render_entities(Render &render);
};