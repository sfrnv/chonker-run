#include <string>

#include <entt/entt.hpp>

#ifndef RENDER_H
#define RENDER_H

#include "Render.hpp"

#endif

#include "AABB.hpp"
#include "Geometry.hpp"

constexpr auto SCALING_FACTOR = 4;

constexpr auto STONE_PIXEL = 0xFF555555;
constexpr auto GRASS_PIXEL = 0xFF00FF00;
constexpr auto WATER_PIXEL = 0xFFFF0000;
constexpr auto SAND_PIXEL = 0xFF00FFFF;
constexpr auto BRICK_PIXEL = 0xFF0000FF;
constexpr auto CRATE_PIXEL = 0xFF004F7D;
constexpr auto LAVA_PIXEL = 0xFF00AAFF;

// TODO: replace with transform
class position : public geom::Point<float> {};

class velocity : public geom::Vector<float> {};

class acceleration : public geom::Vector<float> {};

class force : public geom::Vector<float> {};

struct body {
  unsigned int node;
  float inverse_mass; // Inverse mass
  bool moved;
};

using focus = bool;

struct sprite {
  SDL_Rect tile;
  int layer;
};

class World {
public:
  int width, height;
  bool updated;

  World(const std::initializer_list<std::string> &paths);
  ~World(){};

  void update(Render &render);

private:
  entt::registry registry;
  aabb::Tree tree;
  int layers;
  bool show_tree;

  void load_tiles(int layer, const std::string &path);
  void handle_input();
  void calc_acceleration();
  void calc_velocity();
  void calc_position();
  void detect_collisions();
  void focus_camera(Render &render);
  void render_entities(Render &render);
  void render_tree(Render &render);
};

void impulse_correct(const aabb::AABB &aabb1, const aabb::AABB &aabb2,
                     velocity &v1, velocity &v2, const body &b1,
                     const body &b2);

void projection_correct(position &p1, aabb::AABB &aabb1, aabb::AABB &aabb2);

void projection_correct(position &p1, position &p2, aabb::AABB &aabb1,
                        aabb::AABB &aabb2, const body &b1, const body &b2);