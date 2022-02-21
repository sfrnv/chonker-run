#include <string>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_rect.h>

#include <entt/entt.hpp>

class Render {
public:
  bool updated = false;

  Render(int width, int height, const std::string &title,
         const std::string &path);
  ~Render();

  void present();
  void set_title(const std::string &);
  void update(const SDL_Rect &pos, const SDL_Rect &tile, SDL_Texture *texture);
  void update(const SDL_Rect &pos, const SDL_Rect &tile);
  void draw_frame(int x1, int y1, int x2, int y2, unsigned int color);

  SDL_Rect viewport;

private:
  SDL_Window *window;     // TODO: wrap with unique_ptr
  SDL_Renderer *renderer; // TODO: wrap with unique_ptr
  SDL_Texture *texture;   // TODO: wrap with unique_ptr
};