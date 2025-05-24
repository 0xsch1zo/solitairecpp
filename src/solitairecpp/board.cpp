#include <print>
#include <random>
#include <ranges>
#include <solitairecpp/board.hpp>
#include <solitairecpp/cards.hpp>

namespace solitairecpp {

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

std::expected<Tableau::CardPosition, Error>
Tableau::search(const CardCode &code) const {
  for (const auto [i, cardRow] :
       std::views::zip(std::views::iota(0ULL), tableau_)) {
    auto position = cardRow.search(code);
    if (position) {
      return CardPosition{.cardIndex = position.value().cardIndex,
                          .cardRowIndex = i};
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

ReserveStack::ReserveStack(StartReserveStackCards &&cards) {
  stack_.reserve(cards.size());
  stack_.insert(stack_.end(), cards.begin(), cards.end());
}

BoardElements::BoardElements() {
  Cards deck = buildDeck();

  StartTableauCards tabelauCards;
  std::copy(deck.begin(), deck.begin() + tabelauCards.size(),
            tabelauCards.begin());
  tableau_ = std::make_unique<Tableau>(std::move(tabelauCards));

  StartReserveStackCards reserveStackCards;
  std::copy(deck.begin() + tabelauCards.size(), deck.end(),
            reserveStackCards.begin());
  reserveStack_ = std::make_unique<ReserveStack>(std::move(reserveStackCards));
}

ft::Component BoardElements::component() const {
  // Add the rest of board elements
  return tableau_->component();
}

Cards BoardElements::buildDeck() {
  const auto deckSize = 52;
  Cards deck;
  deck.reserve(deckSize);
  for (size_t i{}; i < static_cast<size_t>(CardValue::Count); i++) {
    for (size_t j{}; j < static_cast<size_t>(CardType::Count); j++) {
      auto value = static_cast<CardValue>(i);
      auto type = static_cast<CardType>(j);
      deck.emplace_back(value, type,
                        "test " + std::to_string(i) + " " + std::to_string(j));
    }
  }

  std::random_device rd;
  std::mt19937 gen{rd()};

  // std::shuffle(deck.begin(), deck.end(), gen);

  return deck;
}

std::expected<CardPosition, Error>
BoardElements::search(const CardCode &code) const {
  auto tableauPos = tableau_->search(code);
  if (tableauPos)
    return tableauPos.value();

  return std::unexpected(ErrorCardPositionNotFound(code).error());
}

CardPosition::CardPosition(const Tableau::CardPosition &pos)
    : tableauPositon_{pos}, section_{BoardSection::Tableau} {}

BoardSection CardPosition::section() { return section_; }

std::expected<Tableau::CardPosition, Error> CardPosition::atTableau() const {
  if (section_ != BoardSection::Tableau)
    return std::unexpected(ErrorWrongSection(section_).error());

  return tableauPositon_;
}

} // namespace solitairecpp
