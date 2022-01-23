#include "Game.hpp"

int main() {
  Game game("chonker-run", "data/sprite_sheet_big_tiles.png",
            "data/water_test_level.png");
  game.run();
}