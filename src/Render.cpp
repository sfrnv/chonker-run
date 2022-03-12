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
  if ((renderer = SDL_CreateRenderer(
           window, -1,
           SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC |
               SDL_RENDERER_TARGETTEXTURE)) == nullptr) {
    throw std::runtime_error(SDL_GetError());
  }
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  if (SDL_Surface *image = IMG_Load(path.c_str())) {
    if ((texture = SDL_CreateTextureFromSurface(renderer, image)) == nullptr) {
      throw std::runtime_error(SDL_GetError());
    }
  }
  viewport = SDL_FRect{.0f, .0f, (float)width, (float)height};
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

void Render::update(const SDL_FRect &pos, const SDL_Rect &tile,
                    SDL_Texture *texture) {
  // if (SDL_HasIntersection(&viewport, &pos)) {
  if (!(pos.x >= viewport.x + viewport.w || pos.y >= viewport.y + viewport.h ||
        pos.x + pos.w <= viewport.x || pos.y + pos.h <= viewport.y)) {
    SDL_FRect result{pos.x - viewport.x, pos.y - viewport.y, pos.w, pos.h};
    SDL_RenderCopyF(renderer, texture, &tile, &result);
    updated = true;
  }
}

void Render::update(const SDL_FRect &pos, const SDL_Rect &tile) {
  update(pos, tile, texture);
}

void Render::draw_frame(int x1, int y1, int x2, int y2, unsigned int color) {
  SDL_Rect pos{x1, y1, x2 - x1, y2 - y1};
  // if (SDL_HasIntersection(&viewport, &pos)) {
  if (!(pos.x >= viewport.x + viewport.w || pos.y >= viewport.y + viewport.h ||
        pos.x + pos.w <= viewport.x || pos.y + pos.h <= viewport.y)) {
    SDL_SetRenderDrawColor(renderer, color & 0xFF, (color >> 8) & 0xFF,
                           (color >> 16) & 0xFF, (color >> 24) & 0xFF);
    SDL_RenderDrawLine(renderer, x1 - viewport.x, y1 - viewport.y,
                       x1 - viewport.x, y2 - viewport.y);
    SDL_RenderDrawLine(renderer, x1 - viewport.x, y1 - viewport.y,
                       x2 - viewport.x, y1 - viewport.y);
    SDL_RenderDrawLine(renderer, x1 - viewport.x, y2 - viewport.y,
                       x2 - viewport.x, y2 - viewport.y);
    SDL_RenderDrawLine(renderer, x2 - viewport.x, y1 - viewport.y,
                       x2 - viewport.x, y2 - viewport.y);
    updated = true;
  }
}