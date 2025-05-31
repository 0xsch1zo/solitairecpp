#include <solitairecpp/board.hpp>
#include <solitairecpp/move_manager.hpp>
#include <stdexcept>

namespace solitairecpp {

bool MoveManager::isRollbackBlocked() const { return history_.empty(); }

void MoveManager::rollback() {
  if (history_.empty())
    return;

  auto transaction = history_.back();
  if (std::holds_alternative<Tableau::CardPosition>(transaction.to) &&
      std::holds_alternative<Tableau::CardPosition>(transaction.from)) {
    rollbackHelper(std::get<Tableau::CardPosition>(transaction.to),
                   std::get<Tableau::CardPosition>(transaction.from));
  } else if (std::holds_alternative<Foundations::CardPosition>(
                 transaction.to) &&
             std::holds_alternative<Tableau::CardPosition>(transaction.from)) {
    rollbackHelper(std::get<Foundations::CardPosition>(transaction.to),
                   std::get<Tableau::CardPosition>(transaction.from));
  } else if (std::holds_alternative<Tableau::CardPosition>(transaction.to) &&
             std::holds_alternative<ReserveStack::CardPosition>(
                 transaction.from)) {
    rollbackHelper(std::get<Tableau::CardPosition>(transaction.to),
                   std::get<ReserveStack::CardPosition>(transaction.from));
  } else if (std::holds_alternative<Foundations::CardPosition>(
                 transaction.to) &&
             std::holds_alternative<ReserveStack::CardPosition>(
                 transaction.from)) {
    rollbackHelper(std::get<Foundations::CardPosition>(transaction.to),
                   std::get<ReserveStack::CardPosition>(transaction.from));
  } else {
    throw std::runtime_error("Illegal rollback operation");
  }
}

void MoveManager::rollbackHelper(const Tableau::CardPosition &from,
                                 const Tableau::CardPosition &to) {
  auto cards = board_.tableau().getCardsFrom(from);
  if (!cards)
    throw std::runtime_error(cards.error()->what());

  auto deleteSuccess = board_.tableau().deleteFrom(from);
  if (!deleteSuccess)
    throw std::runtime_error(deleteSuccess.error()->what());

  auto appendSuccess =
      board_.tableau().appendToRollback({to.cardRowIndex}, cards.value());
  if (!appendSuccess)
    throw std::runtime_error(appendSuccess.error()->what());
}

void MoveManager::rollbackHelper(const Foundations::CardPosition &from,
                                 const Tableau::CardPosition &to) {
  auto card = board_.foundations().acquireCard(from);
  if (!card)
    throw std::runtime_error(card.error()->what());

  auto appendSuccess =
      board_.tableau().appendToRollback({to.cardRowIndex}, {card.value()});
  if (!appendSuccess)
    throw std::runtime_error(appendSuccess.error()->what());
}

void MoveManager::rollbackHelper(const Tableau::CardPosition &from,
                                 const ReserveStack::CardPosition &to) {
  auto card = board_.tableau().getCardsFrom(from);
  if (!card)
    throw std::runtime_error(card.error()->what());

  auto deleteSuccess = board_.tableau().deleteFrom(from);
  if (!deleteSuccess)
    throw std::runtime_error(card.error()->what());

  auto setSuccess = board_.reserveStack().setTopCard(card.value().at(0));
  if (!setSuccess)
    throw std::runtime_error(setSuccess.error()->what());
}

void MoveManager::rollbackHelper(const Foundations::CardPosition &from,
                                 const ReserveStack::CardPosition &to) {
  auto card = board_.foundations().acquireCard(from);
  if (!card)
    throw std::runtime_error(card.error()->what());

  auto setSuccess = board_.reserveStack().setTopCard(card.value());
  if (!setSuccess)
    throw std::runtime_error(setSuccess.error()->what());
}
}; // namespace solitairecpp
