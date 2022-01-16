#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <SDL.h>
#include <SDL_image.h>

#include <entt/entt.hpp>

constexpr auto TICKS_PER_SEC = 60;
constexpr auto TICKS_PER_NSEC = TICKS_PER_SEC / 1e9;

struct position {
  float x;
  float y;
};

struct velocity {
  float dx;
  float dy;
};

void update(entt::registry &registry) {
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

void render_screen(int x, int y) {}

int main() {
  using namespace std::chrono;
  // using namespace std::chrono_literals;

  SDL_Init(SDL_INIT_EVERYTHING);
  IMG_Init(IMG_INIT_JPG);

  SDL_Window *window = SDL_CreateWindow("SDL2Test", SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
  SDL_Surface *image = IMG_Load("data/background.jpg");

  if (window && image) {
    entt::registry registry;

    auto previous = high_resolution_clock::now();
    auto timer = high_resolution_clock::now();
    auto ticks = 0;
    auto delta = 0.0;

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

    SDL_Renderer *renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    // SDL_RenderClear(renderer);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, image);

    // Enter the main loop. Press any key or hit the x to exit.
    for (; event.type != SDL_QUIT && event.type != SDL_KEYDOWN;
         SDL_PollEvent(&event)) {
      auto current = high_resolution_clock::now();
      delta += duration_cast<nanoseconds>(current - previous).count() *
               TICKS_PER_NSEC;

      for (previous = current; delta >= 1.0; --delta) {
        update(registry);
        render_screen(10, 10);
        ticks++;
      }

      if (current - timer >= seconds{1}) {
        SDL_SetWindowTitle(window, ("ticks: " + std::to_string(ticks)).c_str());
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        timer += seconds{1};
        ticks = 0;
      }

      SDL_Delay(10);
    }

    SDL_DestroyRenderer(renderer);

  } else {
    std::cerr << "Error creating the window or opening image: "
              << SDL_GetError() << "\n";
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
}