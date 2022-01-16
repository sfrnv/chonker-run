#include <chrono>
#include <exception>
#include <iostream>

#include "Game.hpp"

struct position {
  float x;
  float y;
};

struct velocity {
  float dx;
  float dy;
};

Game::Game(std::string header, std::string background) {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    throw std::runtime_error(SDL_GetError());
  }
  if ((IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG) != IMG_INIT_JPG) {
    throw std::runtime_error(SDL_GetError());
  }
  if ((window = SDL_CreateWindow(header.c_str(), SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED, 640, 480, 0)) ==
      nullptr) {
    throw std::runtime_error(SDL_GetError());
  }
  if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE)) ==
      nullptr) {
    throw std::runtime_error(SDL_GetError());
  }
  background_path = background;
}

Game::~Game() {
  if (renderer)
    SDL_DestroyRenderer(renderer);
  if (window)
    SDL_DestroyWindow(window);
  IMG_Quit();
  SDL_Quit();
}

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

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  if (SDL_Surface *image = IMG_Load(background_path.c_str())) {
    SDL_RenderCopy(renderer, SDL_CreateTextureFromSurface(renderer, image),
                   NULL, NULL);
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
      // render_screen(10, 10);
      SDL_RenderPresent(renderer);
      ticks++;
    }

    if (current - timer >= seconds{1}) {
      SDL_SetWindowTitle(window, ("ticks: " + std::to_string(ticks)).c_str());
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