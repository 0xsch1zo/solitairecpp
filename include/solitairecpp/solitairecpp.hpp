#pragma once

#include <memory>
#include <solitairecpp/cards.hpp>

namespace solitairecpp {

class Game {
public:
  // Add modes here
  Game();

  void mainLoop();

private:
  Cards buildDeck();
  std::unique_ptr<ReserveStack> reserveStack_ = nullptr;
  std::unique_ptr<Tableau> tableau_ = nullptr;
};

} // namespace solitairecpp
