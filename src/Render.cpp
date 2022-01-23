#include <cmath>
#include <exception>

#ifndef RENDER_H
#define RENDER_H

#include "Render.hpp"

#endif

Render::Render(int width, int height, const std::string &title,
               const std::string &path) {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    throw std::runtime_error(SDL_GetError());
  }
  if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
    throw std::runtime_error(SDL_GetError());
  }
  if ((window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED, width, height, 0)) ==
      nullptr) {
    throw std::runtime_error(SDL_GetError());
  }
  if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE)) ==
      nullptr) {
    throw std::runtime_error(SDL_GetError());
  }
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  if (SDL_Surface *image = IMG_Load(path.c_str())) {
    if ((texture = SDL_CreateTextureFromSurface(renderer, image)) == nullptr) {
      throw std::runtime_error(SDL_GetError());
    }
  }
  viewport = SDL_Rect{0, 0, width, height};
}

Render::~Render() {
  if (renderer)
    SDL_DestroyRenderer(renderer);
  if (window)
    SDL_DestroyWindow(window);
  IMG_Quit();
  SDL_Quit();
}

void Render::present() {
  SDL_RenderPresent(renderer);
  SDL_RenderClear(renderer);
}

void Render::set_title(const std::string &text) {
  SDL_SetWindowTitle(window, text.c_str());
}

void Render::update(const SDL_Rect &pos, const SDL_Rect &tile,
                    SDL_Texture *texture) {
  if (SDL_HasIntersection(&viewport, &pos)) {
    SDL_Rect result{pos.x - viewport.x, pos.y - viewport.y, pos.w, pos.h};
    SDL_RenderCopy(renderer, texture, &tile, &result);
    updated = true;
  }
}

void Render::update(const SDL_Rect &pos, const SDL_Rect &tile) {
  update(pos, tile, texture);
}