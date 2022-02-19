#include "World.hpp"
#include <cmath>

Uint32 get_pixel32(SDL_Surface *surface, int x, int y) {
  // Convert the pixels to 32 bit
  Uint32 *pixels = (Uint32 *)surface->pixels;

  // Get the requested pixel
  return pixels[(y * surface->w) + x];
}

inline auto upscale(int a) { return a << SCALING_FACTOR; }

World::World(const std::string &path)
    : width{0}, height{0}, updated{false}, tree{1.0f, 256} {
  if (SDL_Surface *image = IMG_Load(path.c_str())) {
    width = upscale(image->w);
    height = upscale(image->h);
    // Temporary entity with camera focus:
    const auto entity = registry.create();
    registry.emplace<position>(entity, .0f, .0f);
    registry.emplace<body>(entity,
                           tree.add(entity, aabb::AABB{upscale(0), upscale(0),
                                                       upscale(1), upscale(1)}),
                           0, true);
    registry.emplace<velocity>(entity, .0f, .0f);
    registry.emplace<focus>(entity, true);
    registry.emplace<SDL_Rect>(entity, SDL_Rect{
                                           16,
                                           224,
                                           upscale(1),
                                           upscale(1),
                                       });
    load_tiles(image);
  }
}

void World::load_tiles(SDL_Surface *image) {
  for (auto y = 0; y < image->w; ++y) {
    for (auto x = 0; x < image->h; ++x) {
      switch (get_pixel32(image, x, y)) {
      case STONE_PIXEL: // stone
      {
        const auto entity = registry.create();
        registry.emplace<position>(entity, (float)upscale(x),
                                   (float)upscale(y));
        registry.emplace<body>(
            entity,
            tree.add(entity, aabb::AABB{upscale(x), upscale(y), upscale(x + 1),
                                        upscale(y + 1)}),
            1, true);
        registry.emplace<velocity>(entity, .0f, .0f);
        registry.emplace<SDL_Rect>(entity,
                                   SDL_Rect{16, 0, upscale(1), upscale(1)});
        break;
      }
      case GRASS_PIXEL: // grass
      {
        const auto entity = registry.create();
        registry.emplace<position>(entity, (float)upscale(x),
                                   (float)upscale(y));
        registry.emplace<SDL_Rect>(entity,
                                   SDL_Rect{32, 0, upscale(1), upscale(1)});
        break;
      }
      case WATER_PIXEL: // water
      {
        const auto entity = registry.create();
        registry.emplace<position>(entity, (float)upscale(x),
                                   (float)upscale(y));
        registry.emplace<SDL_Rect>(entity,
                                   SDL_Rect{0, 32, upscale(1), upscale(1)});
        break;
      }
      case SAND_PIXEL: // sand
      {
        const auto entity = registry.create();
        registry.emplace<position>(entity, (float)upscale(x),
                                   (float)upscale(y));
        registry.emplace<SDL_Rect>(entity,
                                   SDL_Rect{48, 0, upscale(1), upscale(1)});
        break;
      }
      case BRICK_PIXEL: // brick
      {
        const auto entity = registry.create();
        registry.emplace<position>(entity, (float)upscale(x),
                                   (float)upscale(y));
        registry.emplace<body>(
            entity,
            tree.add(entity, aabb::AABB{upscale(x), upscale(y), upscale(x + 1),
                                        upscale(y + 1)}),
            0, true);
        registry.emplace<velocity>(entity, .0f, .0f);
        registry.emplace<SDL_Rect>(entity,
                                   SDL_Rect{64, 0, upscale(1), upscale(1)});
        break;
      }
      case LAVA_PIXEL: // lava
      {
        const auto entity = registry.create();
        registry.emplace<position>(entity, (float)upscale(x),
                                   (float)upscale(y));
        registry.emplace<SDL_Rect>(entity,
                                   SDL_Rect{0, 48, upscale(1), upscale(1)});
        break;
      }
      default: // void
        break;
      }
    }
  }
}

void World::update(Render &render) {
  handle_input();
  move_entities();
  detect_collistions();
  focus_camera(render);
  render_entities(render);
}

void World::handle_input() {
  auto view = registry.view<velocity, focus>();
  SDL_Event event;
  view.each([&](auto &vel, auto &focus) {
    while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_KEYDOWN, SDL_KEYUP) >
           0) {
      switch (event.type) {
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_a:
        case SDLK_LEFT:
          vel.dx = -1.0f;
          break;
        case SDLK_d:
        case SDLK_RIGHT:
          vel.dx = 1.0f;
          break;
        case SDLK_w:
        case SDLK_UP:
          vel.dy = -1.0f;
          break;
        case SDLK_s:
        case SDLK_DOWN:
          vel.dy = 1.0f;
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
          if (vel.dx < .0f)
            vel.dx = .0f;
          break;
        case SDLK_d:
        case SDLK_RIGHT:
          if (vel.dx > .0f)
            vel.dx = .0f;
          break;
        case SDLK_w:
        case SDLK_UP:
          if (vel.dy < .0f)
            vel.dy = .0f;
          break;
        case SDLK_s:
        case SDLK_DOWN:
          if (vel.dy > .0f)
            vel.dy = .0f;
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

void World::move_entities() {
  auto view = registry.view<position, velocity, body>();
  view.each([&](auto &pos, auto &vel, auto &body) {
    pos.x += vel.dx;
    pos.y += vel.dy;
    tree[body.node].aabb.x1 += vel.dx;
    tree[body.node].aabb.y1 += vel.dy;
    tree[body.node].aabb.x2 += vel.dx;
    tree[body.node].aabb.y2 += vel.dy;
    if (vel.dx != 0 || vel.dy != 0)
      body.moved = true;
  });
}

void World::detect_collistions() {
  tree.update();
  auto view = registry.view<position, body>();
  view.each([&](auto &p, auto &b) {
    if (b.moved) {
      std::vector<entt::entity> collisions = tree.query(b.node);
      if (collisions.empty()) {
        b.moved = false;
      } else {
        for (auto &o : collisions) {
          if (b.weight > view.get<body>(o).weight) {
            correct(view.get<position>(o), tree[view.get<body>(o).node].aabb,
                    tree[b.node].aabb);
          } else if (b.weight < view.get<body>(o).weight) {
            correct(p, tree[b.node].aabb, tree[view.get<body>(o).node].aabb);
          view.get<body>(o).moved = true;
          } else {
            correct(p, view.get<position>(o), tree[b.node].aabb,
                    tree[view.get<body>(o).node].aabb);
          view.get<body>(o).moved = true;
          }
        }
      }
    }
  });
}

void correct(position &p1, aabb::AABB &aabb1, aabb::AABB &aabb2) {
  auto center1_x = aabb1.x1 + (aabb1.x2 - aabb1.x1) / 2;
  auto center1_y = aabb1.y1 + (aabb1.y2 - aabb1.y1) / 2;
  auto center2_x = aabb2.x1 + (aabb2.x2 - aabb2.x1) / 2;
  auto center2_y = aabb2.y1 + (aabb2.y2 - aabb2.y1) / 2;
  auto vector_x = center1_x - center2_x;
  auto vector_y = center1_y - center2_y;
  auto dx = vector_x > 0 ? aabb1.x1 - aabb2.x2 : aabb1.x2 - aabb2.x1;
  auto dy = vector_y > 0 ? aabb1.y1 - aabb2.y2 : aabb1.y2 - aabb2.y1;
  if (std::abs(vector_x) < std::abs(vector_y)) {
    p1.y -= dy;
    aabb1.y1 -= dy;
    aabb1.y2 -= dy;
  } else if (std::abs(vector_x) > std::abs(vector_y)) {
    p1.x -= dx;
    aabb1.x1 -= dx;
    aabb1.x2 -= dx;
  }
}

void correct(position &p1, position &p2, aabb::AABB &aabb1, aabb::AABB &aabb2) {
  auto center1_x = aabb1.x1 + (aabb1.x2 - aabb1.x1) / 2;
  auto center1_y = aabb1.y1 + (aabb1.y2 - aabb1.y1) / 2;
  auto center2_x = aabb2.x1 + (aabb2.x2 - aabb2.x1) / 2;
  auto center2_y = aabb2.y1 + (aabb2.y2 - aabb2.y1) / 2;
  auto vector_x = center1_x - center2_x;
  auto vector_y = center1_y - center2_y;
  auto dx = (vector_x > 0 ? aabb1.x1 - aabb2.x2 : aabb1.x2 - aabb2.x1) / 2;
  auto dy = (vector_y > 0 ? aabb1.y1 - aabb2.y2 : aabb1.y2 - aabb2.y1) / 2;
  if (std::abs(vector_x) < std::abs(vector_y)) {
    p1.y -= dy;
    p2.y += dy;
    aabb1.y1 -= dy;
    aabb1.y2 -= dy;
    aabb2.y1 += dy;
    aabb2.y2 += dy;
  } else if (std::abs(vector_x) > std::abs(vector_y)) {
    p1.x -= dx;
    p2.x += dx;
    aabb1.x1 -= dx;
    aabb1.x2 -= dx;
    aabb2.x1 += dx;
    aabb2.x2 += dx;
  } else {
    p1.y -= dy;
    p2.y += dy;
    aabb1.y1 -= dy;
    aabb1.y2 -= dy;
    aabb2.y1 += dy;
    aabb2.y2 += dy;
    p1.x -= dx;
    p2.x += dx;
    aabb1.x1 -= dx;
    aabb1.x2 -= dx;
    aabb2.x1 += dx;
    aabb2.x2 += dx;
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
  auto view = registry.view<position, SDL_Rect>();
  view.each([&](auto &pos, auto &tile) {
    render.update(SDL_Rect{(int)pos.x, (int)pos.y, upscale(1), upscale(1)},
                  tile);
  });
}