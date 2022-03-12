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
        auto pos = geom::Point{(float)upscale(x), (float)upscale(y)};
        auto dim = geom::Point{(float)upscale(1), (float)upscale(1)};
        switch (get_pixel32(image, x, y)) {
        case STONE_PIXEL: // stone
        {
          const auto entity = registry.create();
          registry.emplace<position>(entity, geom::Point{pos});
          registry.emplace<body>(entity, tree.add(entity, aabb::AABB{pos, dim}),
                                 .0f, true);
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
          registry.emplace<position>(entity, geom::Point{pos});
          registry.emplace<sprite>(
              entity, SDL_Rect{32, 0, upscale(1), upscale(1)}, layer);
          break;
        }
        case WATER_PIXEL: // water
        {
          const auto entity = registry.create();
          registry.emplace<position>(entity, geom::Point{pos});
          registry.emplace<sprite>(
              entity, SDL_Rect{0, 32, upscale(1), upscale(1)}, layer);
          break;
        }
        case SAND_PIXEL: // sand
        {
          const auto entity = registry.create();
          registry.emplace<position>(entity, geom::Point{pos});
          registry.emplace<sprite>(
              entity, SDL_Rect{48, 0, upscale(1), upscale(1)}, layer);
          break;
        }
        case BRICK_PIXEL: // brick
        {
          const auto entity = registry.create();
          registry.emplace<position>(entity, geom::Point{pos});
          registry.emplace<body>(entity, tree.add(entity, aabb::AABB{pos, dim}),
                                 .0f, true);
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
          registry.emplace<position>(entity, geom::Point{pos});
          registry.emplace<body>(entity, tree.add(entity, aabb::AABB{pos, dim}),
                                 .02f, true);
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
          registry.emplace<position>(entity, geom::Point{pos});
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
  detect_collisions();
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
      vel.x += acc.x - (vel.x * 0.1f); // TODO: remove slowdown
    if (std::abs(vel.y) < 0.001f)
      vel.y = acc.y;
    else
      vel.y += acc.y - (vel.y * 0.1f); // TODO: remove slowdown
  });
}

void World::calc_position() {
  auto view = registry.view<position, velocity, body>();
  view.each([&](auto &pos, auto &vel, auto &body) {
    pos += vel;
    tree[body.node].aabb.pos += vel;
    if (vel != geom::Vector{.0f, .0f})
      body.moved = true;
  });
}

void World::detect_collisions() {
  tree.update();
  auto view = registry.view<position, velocity, body>();
  view.each([&](auto &pos, auto &vel, auto &bod) {
    if (bod.moved) {
      std::vector<entt::entity> collisions = tree.query(bod.node);
      if (collisions.empty()) {
        bod.moved = false;
      } else {
        for (auto &entity : collisions) {
          auto &pos1 = view.get<position>(entity);
          auto &vel1 = view.get<velocity>(entity);
          auto &bod1 = view.get<body>(entity);
          projection_correct(pos, pos1, tree[bod.node].aabb,
                             tree[bod1.node].aabb, bod, bod1);
          if (bod1.inverse_mass > 0)
            impulse_correct(tree[bod.node].aabb, tree[bod1.node].aabb, vel,
                            vel1, bod, bod1);
        }
      }
    }
  });
}

void impulse_correct(const aabb::AABB &aabb1, const aabb::AABB &aabb2,
                     velocity &v1, velocity &v2, const body &b1,
                     const body &b2) {
  velocity normal{(aabb1.center() - aabb2.center()).normalize()};
  float e = 1.0f; // TODO replace it with body field
  auto normilized =
      -(1 + e) * (normal * (v1 - v2)); // v1 - v2 is relative speed
  auto impulse = normilized / (b1.inverse_mass + b2.inverse_mass);
  v1 += normal * impulse * b1.inverse_mass;
  v2 -= normal * impulse * b2.inverse_mass;
}

void projection_correct(position &p1, position &p2, aabb::AABB &aabb1,
                        aabb::AABB &aabb2, const body &b1, const body &b2) {
  auto overlap = aabb1.overlap(aabb2);
  auto vector = aabb1.center() - aabb2.center();
  auto delta = geom::Vector{vector.x >= 0 ? -overlap.dim.x : overlap.dim.x,
                            vector.y >= 0 ? -overlap.dim.y : overlap.dim.y} /
               (b1.inverse_mass + b2.inverse_mass);
  auto delta1 = delta * b1.inverse_mass;
  auto delta2 = delta * b2.inverse_mass;
  if (std::abs(vector.x) < std::abs(vector.y)) {
    p1.y -= delta1.y;
    p2.y += delta2.y;
    aabb1.pos.y -= delta1.y;
    aabb2.pos.y += delta2.y;
  } else if (std::abs(vector.x) > std::abs(vector.y)) {
    p1.x -= delta1.x;
    p2.x += delta2.x;
    aabb1.pos.x -= delta1.x;
    aabb2.pos.x += delta2.x;
  }
}

void World::focus_camera(Render &render) {
  auto view = registry.view<position, focus>();
  view.each([&](auto &pos, auto &focus) {
    if (focus) {
      render.viewport.x = std::clamp<float>(pos.x - render.viewport.w * 0.5, 0,
                                            width - render.viewport.w);
      render.viewport.y = std::clamp<float>(pos.y - render.viewport.h * 0.5, 0,
                                            height - render.viewport.h);
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
      render.draw_frame(tree[i].aabb.pos.x, tree[i].aabb.pos.y,
                        tree[i].aabb.pos.x + tree[i].aabb.dim.x,
                        tree[i].aabb.pos.y + tree[i].aabb.dim.y, 0xFF00FF00);
    } else {
      render.draw_frame(tree[i].fatten.pos.x, tree[i].fatten.pos.y,
                        tree[i].fatten.pos.x + tree[i].fatten.dim.x,
                        tree[i].fatten.pos.y + tree[i].fatten.dim.y,
                        0xFF0000FF);
    }
  }
}