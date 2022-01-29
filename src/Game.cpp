#include <SDL.h>
#include <chrono>
#include <entt/entt.hpp>

#include "Game.hpp"

void Game::run() {
  using namespace std::chrono;

  auto previous = high_resolution_clock::now();
  auto timer = high_resolution_clock::now();
  auto ticks = 0;
  auto frames = 0;
  auto delta = 0.0;

  // Enter the main loop. Press x to exit.
  while (!SDL_HasEvent(SDL_QUIT)) {
    auto current = high_resolution_clock::now();
    delta +=
        duration_cast<nanoseconds>(current - previous).count() * TICKS_PER_NSEC;

    for (previous = current; delta >= 1.0; --delta) {
      SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
      SDL_PumpEvents();
      world.update(render);
      ticks++;
    }

    // SDL_Delay(10);

    if (render.updated) {
      render.present();
      render.updated = false;
      frames++;
    }

    if (current - timer >= seconds{1}) {
      render.set_title("ticks: " + std::to_string(ticks) +
                       "; frames: " + std::to_string(frames));
      timer += seconds{1};
      ticks = 0;
      frames = 0;
    }
  }
}