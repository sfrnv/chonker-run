#include "World.hpp"

Uint32 get_pixel32(SDL_Surface *surface, int x, int y) {
  // Convert the pixels to 32 bit
  Uint32 *pixels = (Uint32 *)surface->pixels;

  // Get the requested pixel
  return pixels[(y * surface->w) + x];
}

inline auto upscale(int a) { return a << SCALING_FACTOR; }

World::World(const std::string &path) : width{0}, height{0}, updated{false} {
  if (SDL_Surface *image = IMG_Load(path.c_str())) {
    width = upscale(image->w);
    height = upscale(image->h);
    load_tiles(image);
    // Temporary entity with camera focus:
    const auto entity = registry.create();
    registry.emplace<position>(entity, .0f, .0f);
    registry.emplace<velocity>(entity, 1.0f, 1.0f);
    registry.emplace<focus>(entity, true);
    registry.emplace<SDL_Rect>(entity, SDL_Rect{
                                           0,
                                           0,
                                           upscale(1),
                                           upscale(1),
                                       });
  }
}

void World::load_tiles(SDL_Surface *image) {
  for (auto y = 0; y < image->w; ++y) {
    for (auto x = 0; x < image->h; ++x) {
      switch (get_pixel32(image, x, y)) {
      case STONE_PIXEL: // stone
      {
        const auto entity = registry.create();
        registry.emplace<std::string>(entity, std::to_string(x));
        registry.emplace<position>(entity, (float)upscale(x),
                                   (float)upscale(y));
        registry.emplace<velocity>(entity, .0f, .0f);
        registry.emplace<SDL_Rect>(entity,
                                   SDL_Rect{16, 0, upscale(1), upscale(1)});
        break;
      }
      case GRASS_PIXEL: // grass
      {
        const auto entity = registry.create();
        registry.emplace<std::string>(entity, std::to_string(x));
        registry.emplace<position>(entity, (float)upscale(x),
                                   (float)upscale(y));
        registry.emplace<velocity>(entity, .0f, .0f);
        registry.emplace<SDL_Rect>(entity,
                                   SDL_Rect{32, 0, upscale(1), upscale(1)});
        break;
      }
      case WATER_PIXEL: // water
      {
        const auto entity = registry.create();
        registry.emplace<std::string>(entity, std::to_string(x));
        registry.emplace<position>(entity, (float)upscale(x),
                                   (float)upscale(y));
        registry.emplace<velocity>(entity, .0f, .0f);
        registry.emplace<SDL_Rect>(entity,
                                   SDL_Rect{0, 32, upscale(1), upscale(1)});
        break;
      }
      case SAND_PIXEL: // sand
      {
        const auto entity = registry.create();
        registry.emplace<std::string>(entity, std::to_string(x));
        registry.emplace<position>(entity, (float)upscale(x),
                                   (float)upscale(y));
        registry.emplace<velocity>(entity, .0f, .0f);
        registry.emplace<SDL_Rect>(entity,
                                   SDL_Rect{48, 0, upscale(1), upscale(1)});
        break;
      }
      case BRICK_PIXEL: // brick
      {
        const auto entity = registry.create();
        registry.emplace<std::string>(entity, std::to_string(x));
        registry.emplace<position>(entity, (float)upscale(x),
                                   (float)upscale(y));
        registry.emplace<velocity>(entity, .0f, .0f);
        registry.emplace<SDL_Rect>(entity,
                                   SDL_Rect{64, 0, upscale(1), upscale(1)});
        break;
      }
      case LAVA_PIXEL: // lava
      {
        const auto entity = registry.create();
        registry.emplace<std::string>(entity, std::to_string(x));
        registry.emplace<position>(entity, (float)upscale(x),
                                   (float)upscale(y));
        registry.emplace<velocity>(entity, .0f, .0f);
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
  move_entities();
  focus_camera(render);
  render_entities(render);
}

void World::move_entities() {
  auto view = registry.view<position, velocity>();
  view.each([&](auto &pos, auto &vel) {
    pos.x += vel.dx;
    pos.y += vel.dy;
  });
}

void World::focus_camera(Render &render) {
  auto view = registry.view<position, focus>();
  view.each([&](auto &pos, auto &focus) {
    if (focus.focused) {
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