#include <algorithm>
#include <print>
#include <solitairecpp/cards.hpp>
#include <solitairecpp/error.hpp>

namespace solitairecpp {

Card::Card(CardValue value, CardType type, std::string art, bool hidden)
    : value_{value}, type_{type}, art_{art}, hidden_{hidden} {}

std::expected<void, Error> CardRow::appendCards(Cards::iterator begin,
                                                Cards::iterator end) {
  // TODO: Check if card range is valid
  cards_.insert(cards_.end(), begin, end);
  return std::expected<void, Error>();
}

std::expected<Cards, Error> CardRow::getCardsFrom(size_t cardIndex) const {
  if (cardIndex >= cards_.size())
    return std::unexpected(ErrorInvalidCardIndex().error());

  Cards result(cards_.begin() + cardIndex, cards_.end());
  return result;
}

Tableau::Tableau(StartTableauCards &&cards) {
  std::vector cardsVec(cards.begin(), cards.end()); // it's just easier to copy

  size_t rowSize = 1;
  for (auto &cardRow : tableau_) {
    auto err =
        cardRow.appendCards(cardsVec.begin(), cardsVec.begin() + rowSize);
    if (!err) {
      std::println("{}", err.error()->what());
      return;
    }
    cardsVec.erase(cardsVec.begin(), cardsVec.begin() + rowSize);
    rowSize++;
  }
}

ReserveStack::ReserveStack(StartReserveStackCards &&cards) {
  stack_.reserve(cards.size());
  stack_.insert(stack_.end(), cards.begin(), cards.end());
}

} // namespace solitairecpp
