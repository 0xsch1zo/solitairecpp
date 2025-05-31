#include <ranges>
#include <solitairecpp/board.hpp>

namespace solitairecpp {

Tableau::Tableau(StartCards cards, MoveManager &moveManager)
    : moveManager_{moveManager} {
  std::vector cardsVec(cards.begin(), cards.end()); // it's just easier to copy

  size_t rowSize = 1;
  for (auto &cardRow : tableau_) {
    std::vector cardRowCards(cardsVec.begin(), cardsVec.begin() + rowSize);
    cardRowCards.back().show(); // The last card is visible
    cardRow.appendCore(cardRowCards);
    cardsVec.erase(cardsVec.begin(), cardsVec.begin() + rowSize);
    rowSize++;
  }
}

std::expected<Tableau::CardPosition, Error>
Tableau::search(const CardCode &code) const {
  for (const auto [i, cardRow] :
       std::views::zip(std::views::iota(0ULL), tableau_)) {
    auto position = cardRow.search(code);
    if (position) {
      return CardPosition{.cardRowIndex = i,
                          .cardIndex = position.value().cardIndex};
    }
  }
  return std::unexpected(ErrorCardPositionNotFound(code).error());
}

ft::Component Tableau::component() const {
  auto container = ft::Container::Horizontal({});
  for (const auto &cardRow : tableau_)
    container->Add(cardRow.component());
  return container;
}

std::expected<void, Error> Tableau::appendTo(const AppendCardPosition &pos,
                                             const Cards &cards) {
  if (pos.cardRowIndex >= tableau_.size())
    return std::unexpected(ErrorInvalidCardIndex().error());

  auto success = tableau_.at(pos.cardRowIndex).append(cards);
  if (!success)
    return std::unexpected(success.error());

  return std::expected<void, Error>();
}

std::expected<void, Error>
Tableau::appendToRollback(const AppendCardPosition &pos, const Cards &cards) {
  if (pos.cardRowIndex >= tableau_.size())
    return std::unexpected(ErrorInvalidCardIndex().error());

  tableau_.at(pos.cardRowIndex).appendRollback(cards);
  return std::expected<void, Error>();
}

std::expected<bool, Error>
Tableau::isAppendToLegal(const AppendCardPosition &pos, const Cards &cards) {
  if (pos.cardRowIndex >= tableau_.size())
    return std::unexpected(ErrorInvalidCardIndex().error());

  return tableau_.at(pos.cardRowIndex).isAppendLegal(cards);
}

std::expected<void, Error> Tableau::deleteFrom(const CardPosition &pos) {
  if (pos.cardRowIndex >= tableau_.size())
    return std::unexpected(ErrorInvalidCardIndex().error());

  auto success =
      tableau_.at(pos.cardRowIndex).deleteFrom({.cardIndex = pos.cardIndex});
  if (!success)
    return std::unexpected(success.error());
  return std::expected<void, Error>();
}

std::expected<Cards, Error> Tableau::getCardsFrom(const CardPosition &pos) {
  if (pos.cardRowIndex >= tableau_.size())
    return std::unexpected(ErrorInvalidCardIndex().error());

  auto success =
      tableau_.at(pos.cardRowIndex).getCardsFrom({.cardIndex = pos.cardIndex});
  if (!success)
    return std::unexpected(success.error());

  return success.value();
}

bool Tableau::CardPosition::operator==(const CardPosition &other) const {
  return cardRowIndex == other.cardRowIndex && cardIndex == other.cardIndex;
}

bool Tableau::AppendCardPosition::operator==(
    const AppendCardPosition &other) const {
  return cardRowIndex == other.cardRowIndex;
}

}; // namespace solitairecpp
