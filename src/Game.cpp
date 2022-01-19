#include <chrono>
#include <SDL.h>
#include <entt/entt.hpp>

#include "Game.hpp"

struct position {
  float x;
  float y;
};

struct velocity {
  float dx;
  float dy;
};

void Game::run() {
  using namespace std::chrono;

  auto previous = high_resolution_clock::now();
  auto timer = high_resolution_clock::now();
  auto ticks = 0;
  auto delta = 0.0;

  entt::registry registry;

  for (auto i = 0u; i < 10u; ++i) {
    const auto entity = registry.create();
    registry.emplace<std::string>(entity, std::to_string(i));
    registry.emplace<position>(entity, i * 1.f, i * 1.f);
    if (i % 2 == 0) {
      registry.emplace<velocity>(entity, i * .1f, i * .1f);
    } else {
      registry.emplace<velocity>(entity, .0f, .0f);
    }
  }

  SDL_Event event;

  // Enter the main loop. Press any key or hit the x to exit.
  for (; event.type != SDL_QUIT && event.type != SDL_KEYDOWN;
       SDL_PollEvent(&event)) {
    auto current = high_resolution_clock::now();
    delta +=
        duration_cast<nanoseconds>(current - previous).count() * TICKS_PER_NSEC;

    for (previous = current; delta >= 1.0; --delta) {
      update(registry);
      render.update();
      ticks++;
    }

    if (current - timer >= seconds{1}) {
      render.update_title(std::to_string(ticks));
      timer += seconds{1};
      ticks = 0;
    }

    SDL_Delay(10);
  }
}

void Game::update(entt::registry &registry) {
  auto view = registry.view<const std::string, position, velocity>();

  // use a callback
  view.each([&](const auto &name, auto &pos, auto &vel) {
    pos.x += vel.dx;
    pos.y += vel.dy;
  });

  // use an extended callback
  view.each([](const auto entity, const auto &name, auto &pos,
               auto &vel) { /* ... */ });

  // use a range-for
  for (auto [entity, name, pos, vel] : view.each()) {
    // ...
  }

  // use forward iterators and get only the components of interest
  for (auto entity : view) {
    auto &vel = view.get<velocity>(entity);
    // ...
  }
}