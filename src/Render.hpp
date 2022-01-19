#include <string>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_rect.h>

#include <entt/entt.hpp>

class Render {
public:
  Render(std::string header, std::string background);
  ~Render();

  void update();
  void update_title(const std::string &);

private:
  SDL_Window *window;     // TODO: wrap with unique_ptr
  SDL_Renderer *renderer; // TODO: wrap with unique_ptr
  SDL_Texture *texture;   // TODO: wrap with unique_ptr
  SDL_Rect viewport;
};