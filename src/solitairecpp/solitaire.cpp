#include "solitairecpp/board.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <print>
#include <solitairecpp/cards.hpp>
#include <solitairecpp/solitairecpp.hpp>
#include <stdexcept>

namespace solitairecpp {

Game::Game() : moveManager_{boardElements_} {}
void Game::mainLoop() {
  auto screen = ft::ScreenInteractive::Fullscreen();
  auto board = boardElements_.component() |
               ft::CatchEvent(moveManager_.cardSelectedHandler());
  screen.Loop(board);
}

Game::MoveManager::MoveManager(const BoardElements &elements)
    : boardElements_{elements} {}

std::function<bool(ft::Event)> Game::MoveManager::cardSelectedHandler() {
  return [this](ft::Event event) -> bool {
    auto card = CardSerializer::Decode(event.input());
    if (!card)
      return false;

    // Find the thing
    if (!moveInitiated_) {
      auto position = boardElements_.search(card.value());
      if (!position)
        throw std::runtime_error(position.error()->what());

      moveMutex_.lock();
      moveFrom_ = position.value();
      moveInitiated_ = true;
    } else {
      auto position = boardElements_.search(card.value());
      if (!position)
        throw std::runtime_error(position.error()->what());

      if (!moveFrom_)
        throw std::runtime_error(
            "moveFrom_ is not a valid position despite the "
            "move operation being initiated");

      // auto success = Move(moveFrom_.value(), position.value()); // handler
      // error
      throw std::runtime_error("Move worked");

      moveFrom_ = std::nullopt;
      moveMutex_.unlock();
    }
    return true;
  };
}

} // namespace solitairecpp
