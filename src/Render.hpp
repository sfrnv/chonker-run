#include <string>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_rect.h>

#include <entt/entt.hpp>

class Render {
  bool updated = false;
public:
  Render(int width, int height, std::string header, std::string background);
  ~Render();

  void update();
  void add(const SDL_Rect &pos, const SDL_Rect &tile, SDL_Texture *texture);
  void add(const SDL_Rect &pos, const SDL_Rect &tile);
  void update_title(const std::string &);

private:

  SDL_Window *window;     // TODO: wrap with unique_ptr
  SDL_Renderer *renderer; // TODO: wrap with unique_ptr
  SDL_Texture *texture;   // TODO: wrap with unique_ptr
  SDL_Rect viewport;
};