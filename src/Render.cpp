#include <exception>
#include <iostream>

#include "Render.hpp"

constexpr auto WINDOW_WIDTH = 640;
constexpr auto WINDOW_HEIGHT = 480;

Render::Render(std::string header, std::string background) {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    throw std::runtime_error(SDL_GetError());
  }
  if ((IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG) != IMG_INIT_JPG) {
    throw std::runtime_error(SDL_GetError());
  }
  if ((window = SDL_CreateWindow(header.c_str(), SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
                                 WINDOW_HEIGHT, 0)) == nullptr) {
    throw std::runtime_error(SDL_GetError());
  }
  if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE)) ==
      nullptr) {
    throw std::runtime_error(SDL_GetError());
  }
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  if (SDL_Surface *image = IMG_Load(background.c_str())) {
    if ((texture = SDL_CreateTextureFromSurface(renderer, image)) == nullptr) {
      throw std::runtime_error(SDL_GetError());
    }
  }
  viewport = SDL_Rect{0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
}

Render::~Render() {
  if (renderer)
    SDL_DestroyRenderer(renderer);
  if (window)
    SDL_DestroyWindow(window);
  IMG_Quit();
  SDL_Quit();
}

void Render::update() {
  int w, h;
  SDL_RenderClear(renderer);
  SDL_QueryTexture(texture, NULL, NULL, &w, &h);
  if (viewport.x + WINDOW_WIDTH < w) {
    viewport.x += 8;
  } else if (viewport.y + WINDOW_HEIGHT < h) {
    viewport.y += 8;
  }
  SDL_RenderCopy(renderer, texture, &viewport, NULL);
  SDL_RenderPresent(renderer);
}

void Render::update_title(const std::string &text) {
  SDL_SetWindowTitle(window, ("ticks: " + text).c_str());
}