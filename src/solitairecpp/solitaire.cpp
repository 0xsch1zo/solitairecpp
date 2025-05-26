#include "solitairecpp/board.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <print>
#include <solitairecpp/cards.hpp>
#include <solitairecpp/solitairecpp.hpp>
#include <stdexcept>
#include <variant>

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

std::expected<void, Error> Game::MoveManager::Move(const CardPosition &from,
                                                   const CardPosition &to) {
  if (std::holds_alternative<Tableau::CardPosition>(from)) {
    if (std::holds_alternative<Tableau::CardPosition>(to)) {
      auto success = boardElements_.tableau_->deleteFrom(
          std::get<Tableau::CardPosition>(to));
      if (!success)
        throw std::runtime_error(success.error()->what());
    }
  }

  return std::expected<void, Error>();
}

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

      auto success = Move(moveFrom_.value(), position.value()); // handle
      // throw std::runtime_error("Move worked");

      moveFrom_ = std::nullopt;
    }
    return true;
  };
}

} // namespace solitairecpp
