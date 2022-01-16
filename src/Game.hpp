#include <string>

#include <SDL.h>
#include <SDL_image.h>

#include <entt/entt.hpp>

constexpr auto TICKS_PER_SEC = 60;
constexpr auto TICKS_PER_NSEC = TICKS_PER_SEC / 1e9;

class Game {
public:
  Game(std::string header, std::string background);
  ~Game();
  void run();

private:
  SDL_Window *window;     // TODO: wrap with unique_ptr
  SDL_Renderer *renderer; // TODO: wrap with unique_ptr

  std::string background_path;

  void update(entt::registry &);
  void render();
};