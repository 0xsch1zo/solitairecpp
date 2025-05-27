#pragma once

#include <solitairecpp/board.hpp>
#include <solitairecpp/cards.hpp>
#include <solitairecpp/error.hpp>
#include <solitairecpp/move_manager.hpp>

namespace solitairecpp {

class Game {
public:
  // Add modes here
  Game();

  void mainLoop();

private:
  BoardElements boardElements_{};
  MoveManager moveManager_;
};

} // namespace solitairecpp
