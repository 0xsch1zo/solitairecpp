#include "solitairecpp/error.hpp"
#include <expected>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <optional>
#include <print>
#include <solitairecpp/board.hpp>
#include <solitairecpp/move_manager.hpp>
#include <solitairecpp/utils.hpp>
#include <stdexcept>
#include <variant>

namespace solitairecpp {

MoveManager::MoveManager(const Board &elements) : board_{elements} {}

void MoveManager::rollback() {
  /*(auto transaction = history_.back();
  if (std::holds_alternative<Tableau::CardPosition>(transaction.from) &&
      std::holds_alternative<Tableau::AppendCardPosition>(transaction.to)) {
    board_.tableau().
  }*/
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
    endTransaction();
    return std::unexpected(ErrorIllegalMove().error());
  }

  const auto from = moveFrom_.load().value();
  const auto to = moveTo_.load().value();

  if (std::holds_alternative<Tableau::CardPosition>(from) &&
      std::holds_alternative<Tableau::AppendCardPosition>(to)) {
    auto success = moveHelper(std::get<Tableau::CardPosition>(from),
                              std::get<Tableau::AppendCardPosition>(to));
    if (!success) {
      endTransaction();
      return std::unexpected(success.error());
    }
  } else if (std::holds_alternative<Tableau::CardPosition>(from) &&
             std::holds_alternative<Foundations::CardPosition>(to)) {
    auto success = moveHelper(std::get<Tableau::CardPosition>(from),
                              std::get<Foundations::CardPosition>(to));
    if (!success) {
      endTransaction();
      return std::unexpected(success.error());
    }
  } else if (std::holds_alternative<ReserveStack::CardPosition>(from) &&
             std::holds_alternative<Tableau::AppendCardPosition>(to)) {
    auto success = moveHelper(std::get<ReserveStack::CardPosition>(from),
                              std::get<Tableau::AppendCardPosition>(to));
    if (!success) {
      endTransaction();
      return std::unexpected(success.error());
    }
  } else if (std::holds_alternative<ReserveStack::CardPosition>(from) &&
             std::holds_alternative<Foundations::CardPosition>(to)) {
    auto success = moveHelper(std::get<ReserveStack::CardPosition>(from),
                              std::get<Foundations::CardPosition>(to));
    if (!success) {
      endTransaction();
      return std::unexpected(success.error());
    }
  } else {
    endTransaction();
    return std::unexpected(ErrorIllegalMove().error());
  }

  // if (history_.size() < maxHistorySize_)
  //  history_.emplace_back(from, to);
  endTransaction();
  return std::expected<void, Error>();
}

std::expected<void, Error>
MoveManager::moveHelper(const Tableau::CardPosition &from,
                        const Tableau::AppendCardPosition &to) {
  auto cards = board_.tableau().getCardsFrom(from);
  if (!cards)
    throw std::runtime_error(cards.error()->what());

  auto legalSuccess = board_.tableau().isAppendToLegal(to, cards.value());
  if (!legalSuccess)
    throw std::runtime_error(legalSuccess.error()->what());

  if (!legalSuccess.value())
    return std::unexpected(ErrorIllegalMove().error());

  auto deleteSuccess = board_.tableau().deleteFrom(from);
  if (!deleteSuccess)
    throw std::runtime_error(deleteSuccess.error()->what());

  auto appendSuccess = board_.tableau().appendTo(to, cards.value());
  if (!appendSuccess)
    throw std::runtime_error(appendSuccess.error()->what());

  return std::expected<void, Error>();
}

// fix this one
std::expected<void, Error>
MoveManager::moveHelper(const Tableau::CardPosition &from,
                        const Foundations::CardPosition &to) {
  auto card = board_.tableau().getCardsFrom(from);
  if (!card)
    throw std::runtime_error(card.error()->what());

  if (card.value().size() > 1) // cards get set one by one to foundations
    return std::unexpected(ErrorIllegalMove().error());

  auto legalSuccess = board_.foundations().isSetLegal(to, card.value().at(0));
  if (!legalSuccess)
    throw std::runtime_error(card.error()->what());
  if (!legalSuccess.value())
    return std::unexpected(ErrorIllegalMove().error());

  auto deleteSuccess = board_.tableau().deleteFrom(from);
  if (!deleteSuccess)
    throw std::runtime_error(deleteSuccess.error()->what());

  auto appendSuccess = board_.foundations().set(to, card.value().at(0));
  if (!appendSuccess)
    throw std::runtime_error(appendSuccess.error()->what());

  return std::expected<void, Error>();
}

std::expected<void, Error>
MoveManager::moveHelper(const ReserveStack::CardPosition &from,
                        const Tableau::AppendCardPosition &to) {
  auto card = board_.reserveStack().getTopCard();
  if (!card)
    throw std::runtime_error(card.error()->what());

  auto legalSuccess = board_.tableau().isAppendToLegal(to, {card.value()});
  if (!legalSuccess)
    throw std::runtime_error(card.error()->what());

  if (!legalSuccess.value())
    return std::unexpected(ErrorIllegalMove().error());

  auto deleteSuccess = board_.reserveStack().deleteTopCard();
  if (!deleteSuccess)
    throw std::runtime_error(deleteSuccess.error()->what());

  auto appendSuccess = board_.tableau().appendTo(to, {card.value()});
  if (!appendSuccess) {
    throw std::runtime_error(appendSuccess.error()->what());
  }

  return std::expected<void, Error>();
}

std::expected<void, Error>
MoveManager::moveHelper(const ReserveStack::CardPosition &from,
                        const Foundations::CardPosition &to) {
  auto card = board_.reserveStack().getTopCard();
  if (!card)
    throw std::runtime_error(card.error()->what());

  auto legalSuccess = board_.foundations().isSetLegal(to, {card.value()});
  if (!legalSuccess)
    throw std::runtime_error(card.error()->what());

  if (!legalSuccess.value())
    return std::unexpected(ErrorIllegalMove().error());

  auto deleteSuccess = board_.reserveStack().deleteTopCard();
  if (!deleteSuccess)
    throw std::runtime_error(deleteSuccess.error()->what());

  auto appendSuccess = board_.foundations().set(to, {card.value()});
  if (!appendSuccess) {
    throw std::runtime_error(appendSuccess.error()->what());
  }
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

void MoveManager::setMoveTarget(const CardCode &code) {
  auto position = board_.search(code);
  if (!position)
    throw std::runtime_error(position.error()->what());

  setMoveTarget(position.value());
}

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

void MoveManager::endTransaction() {
  moveFrom_ = std::nullopt;
  moveTo_ = std::nullopt;
}

} // namespace solitairecpp
