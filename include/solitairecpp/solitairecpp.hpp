#pragma once

#include <solitairecpp/board.hpp>
#include <solitairecpp/cards.hpp>
#include <solitairecpp/error.hpp>

namespace solitairecpp {

class Game {
public:
  // Add modes here
  Game();

  void mainLoop();

private:
  class MoveManager {
  public:
    MoveManager(const BoardElements &elements);
    std::function<bool(ft::Event)> cardSelectedHandler();

  private:
    std::expected<void, Error> Move(const CardPosition &from,
                                    const CardPosition &to);

  private:
    const BoardElements &boardElements_;
    // use atomics
    bool moveInitiated_{};
    std::optional<CardPosition>
        moveFrom_; // only when move sequence is initiated
  };

private:
  BoardElements boardElements_{};
  MoveManager moveManager_;
};

} // namespace solitairecpp
