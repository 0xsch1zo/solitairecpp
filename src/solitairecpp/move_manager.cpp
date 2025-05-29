#include <expected>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <optional>
#include <print>
#include <solitairecpp/board.hpp>
#include <solitairecpp/move_manager.hpp>
#include <solitairecpp/utils.hpp>
#include <stdexcept>
#include <utility>
#include <variant>

namespace solitairecpp {

MoveManager::MoveManager(const Board &elements) : board_{elements} {}

bool MoveManager::isBeingMoved(const CardCode &code) const {
  const auto &from = moveFrom_.load();
  if (!from.has_value())
    return false;
  auto position = board_.search(code);
  if (!position)
    throw std::runtime_error(position.error()->what());

  if (from.value().index() != position.value().index())
    return false;

  if (std::holds_alternative<Tableau::CardPosition>(from.value()) &&
      std::holds_alternative<Tableau::CardPosition>(position.value()))
    return std::get<Tableau::CardPosition>(from.value()) ==
           std::get<Tableau::CardPosition>(position.value());

  if (std::holds_alternative<Tableau::AppendCardPosition>(from.value()) &&
      std::holds_alternative<Tableau::AppendCardPosition>(position.value()))
    return std::get<Tableau::AppendCardPosition>(from.value()) ==
           std::get<Tableau::AppendCardPosition>(position.value());

  if (std::holds_alternative<ReserveStack::CardPosition>(from.value()))
    return true;

  std::unreachable();
}

bool MoveManager::isTargetError(const CardPosition &pos) const {
  if (!erroneusTarget_.load().has_value())
    return false;
  if (std::holds_alternative<Tableau::AppendCardPosition>(pos) &&
      std::holds_alternative<Tableau::AppendCardPosition>(
          erroneusTarget_.load().value())) {
    return std::get<Tableau::AppendCardPosition>(pos) ==
           std::get<Tableau::AppendCardPosition>(
               erroneusTarget_.load().value());
  }

  return false;
}

bool MoveManager::moveTransactionOpen() const {
  return moveFrom_.load() != std::nullopt;
}

std::expected<void, Error> MoveManager::Move() {
  if (!moveFrom_.load().has_value() || !moveTo_.load().has_value()) {
    moveFrom_ = std::nullopt;
    moveTo_ = std::nullopt;
    return std::unexpected(ErrorIllegalMove().error());
  }

  const auto from = moveFrom_.load().value();
  const auto to = moveTo_.load().value();

  if (std::holds_alternative<Tableau::CardPosition>(from)) {
    if (std::holds_alternative<Tableau::AppendCardPosition>(to)) {
      auto cards =
          board_.tableau().getCardsFrom(std::get<Tableau::CardPosition>(from));
      if (!cards)
        throw std::runtime_error(cards.error()->what());

      auto deleteSuccess =
          board_.tableau().deleteFrom(std::get<Tableau::CardPosition>(from));
      if (!deleteSuccess)
        throw std::runtime_error(deleteSuccess.error()->what());

      auto appendSuccess = board_.tableau().appendTo(
          {.cardRowIndex =
               std::get<Tableau::AppendCardPosition>(to).cardRowIndex},
          cards.value());
      if (!appendSuccess)
        throw std::runtime_error(appendSuccess.error()->what());
    }
  } else if (std::holds_alternative<ReserveStack::CardPosition>(from)) {
    if (std::holds_alternative<Tableau::AppendCardPosition>(to)) {
      auto card = board_.reserveStack().getTopCard();
      if (!card)
        throw std::runtime_error(card.error()->what());

      auto deleteSuccess = board_.reserveStack().deleteTopCard();
      if (!deleteSuccess)
        throw std::runtime_error(deleteSuccess.error()->what());

      auto appendSuccess = board_.tableau().appendTo(
          {.cardRowIndex =
               std::get<Tableau::AppendCardPosition>(to).cardRowIndex},
          {card.value()});
      if (!appendSuccess)
        throw std::runtime_error(appendSuccess.error()->what());
    }
  } else {
    moveFrom_ = std::nullopt;
    moveTo_ = std::nullopt;
    return std::unexpected(ErrorIllegalMove().error());
  }

  moveFrom_ = std::nullopt;
  moveTo_ = std::nullopt;
  return std::expected<void, Error>();
}

void MoveManager::setMoveTarget(const CardPosition &pos) {
  if (moveTo_.load() != std::nullopt)
    erroneusTarget_ = pos;

  // add logic
  moveTo_ = pos;
  auto success = Move();
  if (!success)
    erroneusTarget_ = pos;
};

void MoveManager::cardSelected(const CardCode &code) {
  // another move started so we reset the erroneusTarget_
  if (erroneusTarget_.load().has_value())
    erroneusTarget_ = std::nullopt;

  // Find the thing
  if (moveFrom_.load() == std::nullopt) {
    auto position = board_.search(code);
    if (!position)
      throw std::runtime_error(position.error()->what());

    moveFrom_ = position.value();
  }
}

} // namespace solitairecpp
