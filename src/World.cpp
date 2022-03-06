#include "World.hpp"
#include <cmath>
#include <iostream>

Uint32 get_pixel32(SDL_Surface *surface, int x, int y) {
  // Convert the pixels to 32 bit
  Uint32 *pixels = (Uint32 *)surface->pixels;

  // Get the requested pixel
  return pixels[(y * surface->w) + x];
}

// TODO: replace with template
inline auto upscale(int a) { return a << SCALING_FACTOR; }

World::World(const std::initializer_list<std::string> &paths)
    : width{0}, height{0}, updated{false}, layers{0}, tree{1.0f, 256} {
  for (auto const &i : paths)
    load_tiles(layers++, i);
  // Temporary entity with camera focus:
  const auto entity = registry.create();
  registry.emplace<position>(entity, geom::Point{.0f, .0f});
  registry.emplace<body>(
      entity,
      tree.add(entity, aabb::AABB{(float)upscale(0), (float)upscale(0),
                                  (float)upscale(1), (float)upscale(1)}),
      .1f, true);
  registry.emplace<velocity>(entity, geom::Vector{.0f, .0f});
  registry.emplace<acceleration>(entity, geom::Vector{.0f, .0f});
  registry.emplace<force>(entity, geom::Vector{.0f, .0f});
  registry.emplace<focus>(entity, true);
  registry.emplace<sprite>(entity,
                           SDL_Rect{
                               16,
                               224,
                               upscale(1),
                               upscale(1),
                           },
                           1);
}

void World::load_tiles(int layer, const std::string &path) {
  if (SDL_Surface *image = IMG_Load(path.c_str())) {
    width = upscale(image->w);
    height = upscale(image->h);
    for (auto y = 0; y < image->w; ++y) {
      for (auto x = 0; x < image->h; ++x) {
        auto pos1 = geom::Point{(float)upscale(x), (float)upscale(y)};
        auto pos2 = geom::Point{(float)upscale(x + 1), (float)upscale(y + 1)};
        switch (get_pixel32(image, x, y)) {
        case STONE_PIXEL: // stone
        {
          const auto entity = registry.create();
          registry.emplace<position>(entity, geom::Point{pos1});
          registry.emplace<body>(
              entity, tree.add(entity, aabb::AABB{pos1, pos2}), .0f, true);
          registry.emplace<velocity>(entity, geom::Vector{.0f, .0f});
          registry.emplace<acceleration>(entity, geom::Vector{.0f, .0f});
          registry.emplace<force>(entity, geom::Vector{.0f, .0f});
          registry.emplace<sprite>(
              entity, SDL_Rect{16, 0, upscale(1), upscale(1)}, layer);
          break;
        }
        case GRASS_PIXEL: // grass
        {
          const auto entity = registry.create();
          registry.emplace<position>(entity, geom::Point{pos1});
          registry.emplace<sprite>(
              entity, SDL_Rect{32, 0, upscale(1), upscale(1)}, layer);
          break;
        }
        case WATER_PIXEL: // water
        {
          const auto entity = registry.create();
          registry.emplace<position>(entity, geom::Point{pos1});
          registry.emplace<sprite>(
              entity, SDL_Rect{0, 32, upscale(1), upscale(1)}, layer);
          break;
        }
        case SAND_PIXEL: // sand
        {
          const auto entity = registry.create();
          registry.emplace<position>(entity, geom::Point{pos1});
          registry.emplace<sprite>(
              entity, SDL_Rect{48, 0, upscale(1), upscale(1)}, layer);
          break;
        }
        case BRICK_PIXEL: // brick
        {
          const auto entity = registry.create();
          registry.emplace<position>(entity, geom::Point{pos1});
          registry.emplace<body>(
              entity, tree.add(entity, aabb::AABB{pos1, pos2}), .0f, true);
          registry.emplace<velocity>(entity, geom::Vector{.0f, .0f});
          registry.emplace<acceleration>(entity, geom::Vector{.0f, .0f});
          registry.emplace<force>(entity, geom::Vector{.0f, .0f});
          registry.emplace<sprite>(
              entity, SDL_Rect{64, 0, upscale(1), upscale(1)}, layer);
          break;
        }
        case CRATE_PIXEL: // crate
        {
          const auto entity = registry.create();
          registry.emplace<position>(entity, geom::Point{pos1});
          registry.emplace<body>(
              entity, tree.add(entity, aabb::AABB{pos1, pos2}), .02f, true);
          registry.emplace<velocity>(entity, geom::Vector{.0f, .0f});
          registry.emplace<acceleration>(entity, geom::Vector{.0f, .0f});
          registry.emplace<force>(entity, geom::Vector{.0f, .0f});
          registry.emplace<sprite>(
              entity, SDL_Rect{80, 0, upscale(1), upscale(1)}, layer);
          break;
        }
        case LAVA_PIXEL: // lava
        {
          const auto entity = registry.create();
          registry.emplace<position>(entity, geom::Point{pos1});
          registry.emplace<sprite>(
              entity, SDL_Rect{0, 48, upscale(1), upscale(1)}, layer);
          break;
        }
        default: // void
          break;
        }
      }
    }
  }
}

void World::update(Render &render) {
  handle_input();
  calc_acceleration();
  calc_velocity();
  calc_position();
  detect_collistions();
  focus_camera(render);
  render_entities(render);
  if (show_tree)
    render_tree(render);
}

void World::handle_input() {
  auto view = registry.view<force, focus>();
  SDL_Event event;
  view.each([&](auto &force, auto &focus) {
    while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_KEYDOWN, SDL_KEYUP) >
           0) {
      switch (event.type) {
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_a:
        case SDLK_LEFT:
          force.x = -1.0f;
          break;
        case SDLK_d:
        case SDLK_RIGHT:
          force.x = 1.0f;
          break;
        case SDLK_w:
        case SDLK_UP:
          force.y = -1.0f;
          break;
        case SDLK_s:
        case SDLK_DOWN:
          force.y = 1.0f;
          break;
        case SDLK_p:
          tree.print();
          break;
        default:
          break;
        }
        break;
      case SDL_KEYUP:
        switch (event.key.keysym.sym) {
        case SDLK_a:
        case SDLK_LEFT:
          if (force.x < .0f)
            force.x = .0f;
          break;
        case SDLK_d:
        case SDLK_RIGHT:
          if (force.x > .0f)
            force.x = .0f;
          break;
        case SDLK_w:
        case SDLK_UP:
          if (force.y < .0f)
            force.y = .0f;
          break;
        case SDLK_s:
        case SDLK_DOWN:
          if (force.y > .0f)
            force.y = .0f;
          break;
        case SDLK_t:
          show_tree = !show_tree;
          break;
        default:
          break;
        }
        break;
      default:
        break;
      }
    }
  });
}

void World::calc_acceleration() {
  auto view = registry.view<body, acceleration, force>();
  view.each([&](auto &body, auto &acc, auto &force) {
    acc.x = force.x * body.inverse_mass;
    acc.y = force.y * body.inverse_mass;
  });
}

void World::calc_velocity() {
  auto view = registry.view<velocity, acceleration>();
  view.each([&](auto &vel, auto &acc) {
    if (std::abs(vel.x) < 0.001f)
      vel.x = acc.x;
    else
      vel.x += acc.x - (vel.x / 10); // TODO: remove slowdown
    if (std::abs(vel.y) < 0.001f)
      vel.y = acc.y;
    else
      vel.y += acc.y - (vel.y / 10); // TODO: remove slowdown
  });
}

void World::calc_position() {
  auto view = registry.view<position, velocity, body>();
  view.each([&](auto &pos, auto &vel, auto &body) {
    pos += vel;
    tree[body.node].aabb.p1 += vel;
    tree[body.node].aabb.p2 += vel;
    if (vel != geom::Vector{.0f, .0f})
      body.moved = true;
  });
}

void World::detect_collistions() {
  tree.update();
  auto view = registry.view<position, velocity, body>();
  view.each([&](auto &p, auto &v, auto &b) {
    if (b.moved) {
      std::vector<entt::entity> collisions = tree.query(b.node);
      if (collisions.empty()) {
        b.moved = false;
      } else {
        for (auto &o : collisions) {
          auto &p1 = view.get<position>(o);
          auto &v1 = view.get<velocity>(o);
          auto &b1 = view.get<body>(o);
          if (b.inverse_mass < b1.inverse_mass) {
            projection_correct(p1, tree[b1.node].aabb, tree[b.node].aabb);
          } else if (b.inverse_mass > b1.inverse_mass) {
            projection_correct(p, tree[b.node].aabb, tree[b1.node].aabb);
          } else {
            projection_correct(p, p1, tree[b.node].aabb, tree[b1.node].aabb);
          }
          if (b1.inverse_mass > 0)
            impulse_correct(tree[b.node].aabb, tree[b1.node].aabb, v, v1, b,
                            b1);
        }
      }
    }
  });
}

void impulse_correct(const aabb::AABB &aabb1, const aabb::AABB &aabb2,
                     velocity &v1, velocity &v2, body &b1, body &b2) {
  position center1{(aabb1.p1 + aabb1.p2) / 2};
  position center2{(aabb2.p1 + aabb2.p2) / 2};
  velocity normal{(center1 - center2).normalize()};
  float e = 1.0f; // TODO replace it with body field
  auto normilized =
      -(1 + e) * (normal * (v1 - v2)); // v1 - v2 is relative speed
  auto impulse = normilized / (b1.inverse_mass + b2.inverse_mass);
  v1 += normal * impulse * b1.inverse_mass;
  v2 -= normal * impulse * b2.inverse_mass;
}

void projection_correct(position &p1, aabb::AABB &aabb1, aabb::AABB &aabb2) {
  position center1{(aabb1.p1 + aabb1.p2) / 2};
  position center2{(aabb2.p1 + aabb2.p2) / 2};
  auto vector = center1 - center2;
  auto dx = vector.x > 0 ? aabb1.p1.x - aabb2.p2.x : aabb1.p2.x - aabb2.p1.x;
  auto dy = vector.y > 0 ? aabb1.p1.y - aabb2.p2.y : aabb1.p2.y - aabb2.p1.y;
  if (std::abs(vector.x) < std::abs(vector.y)) {
    p1.y -= dy;
    aabb1.p1.y -= dy;
    aabb1.p2.y -= dy;
  } else if (std::abs(vector.x) > std::abs(vector.y)) {
    p1.x -= dx;
    aabb1.p1.x -= dx;
    aabb1.p2.x -= dx;
  }
}

void projection_correct(position &p1, position &p2, aabb::AABB &aabb1,
                        aabb::AABB &aabb2) {
  position center1{(aabb1.p1 + aabb1.p2) / 2};
  position center2{(aabb2.p1 + aabb2.p2) / 2};
  auto vector = center1 - center2;
  auto dx =
      (vector.x > 0 ? aabb1.p1.x - aabb2.p2.x : aabb1.p2.x - aabb2.p1.x) / 2;
  auto dy =
      (vector.y > 0 ? aabb1.p1.y - aabb2.p2.y : aabb1.p2.y - aabb2.p1.y) / 2;
  if (std::abs(vector.x) <= std::abs(vector.y)) {
    p1.y -= dy;
    p2.y += dy;
    aabb1.p1.y -= dy;
    aabb1.p2.y -= dy;
    aabb2.p1.y += dy;
    aabb2.p2.y += dy;
  }
  if (std::abs(vector.x) >= std::abs(vector.y)) {
    p1.x -= dx;
    p2.x += dx;
    aabb1.p1.x -= dx;
    aabb1.p2.x -= dx;
    aabb2.p1.x += dx;
    aabb2.p2.x += dx;
  }
}

void World::focus_camera(Render &render) {
  auto view = registry.view<position, focus>();
  view.each([&](auto &pos, auto &focus) {
    if (focus) {
      int x_offset = pos.x - render.viewport.w / 2;
      int y_offset = pos.y - render.viewport.h / 2;
      if (x_offset < 0) {
        x_offset = 0;
      } else if (x_offset > width - render.viewport.w) {
        x_offset = width - render.viewport.w;
      }
      if (y_offset < 0) {
        y_offset = 0;
      } else if (y_offset > height - render.viewport.h) {
        y_offset = height - render.viewport.h;
      }
      render.viewport.x = x_offset;
      render.viewport.y = y_offset;
    }
  });
}

void World::render_entities(Render &render) {
  auto view = registry.view<position, sprite>();
  for (auto layer = 0; layer < layers; ++layer) {
    view.each([&](auto &pos, auto &spr) {
      if (spr.layer == layer)
        render.update(
            SDL_FRect{pos.x, pos.y, (float)upscale(1), (float)upscale(1)},
            spr.tile);
    });
  }
}

void World::render_tree(Render &render) {
  for (auto i = 0; i < tree.size(); ++i) {
    if (tree[i].is_leaf()) {
      render.draw_frame(tree[i].aabb.p1.x, tree[i].aabb.p1.y, tree[i].aabb.p2.x,
                        tree[i].aabb.p2.y, 0xFF00FF00);
    } else {
      render.draw_frame(tree[i].fatten.p1.x, tree[i].fatten.p1.y,
                        tree[i].fatten.p2.x, tree[i].fatten.p2.y, 0xFF0000FF);
    }
  }
}