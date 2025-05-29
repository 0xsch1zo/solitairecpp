#include <memory>
#include <print>
#include <random>
#include <ranges>
#include <solitairecpp/board.hpp>
#include <solitairecpp/cards.hpp>
#include <solitairecpp/move_manager.hpp>
#include <stdexcept>

namespace solitairecpp {
Tableau::Tableau(StartCards cards, MoveManager &moveManager)
    : moveManager_{moveManager} {
  std::vector cardsVec(cards.begin(), cards.end()); // it's just easier to copy

  size_t rowSize = 1;
  for (auto &cardRow : tableau_) {
    std::vector cardRowCards(cardsVec.begin(), cardsVec.begin() + rowSize);
    cardRowCards.back().show(); // The last card is visible
    auto err = cardRow.append(cardRowCards);
    if (!err) {
      throw std::runtime_error(err.error()->what());
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

ReserveStack::ReserveStack(StartCards cards) {
  stack_.reserve(cards.size());
  stack_.insert(stack_.end(), cards.begin(), cards.end());
}

Board::Board() : moveManager_{std::make_unique<MoveManager>(*this)} {
  Cards deck = buildDeck();

  Tableau::StartCards tabelauCards =
      [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        return Tableau::StartCards{{deck.at(Is)...}};
      }(std::make_index_sequence<
          Tableau::startCardsSize>{}); // This is just initializing the array
                                       // with elements from the deck. Yes there
                                       // is problably no other simpler way.
  tableau_ = std::make_unique<Tableau>(tabelauCards, *moveManager_);

  ReserveStack::StartCards reserveStackCards =
      [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        return ReserveStack::StartCards{{deck.at(Is)...}};
      }(std::make_index_sequence<ReserveStack::startCardsSize>{});
  reserveStack_ = std::make_unique<ReserveStack>(reserveStackCards);
}

ft::Component Board::component() const {
  // Add the rest of board elements
  return tableau_->component() |
         ft::CatchEvent(moveManager_->cardSelectedHandler());
}

Cards Board::buildDeck() {
  const auto deckSize = 52;
  Cards deck;
  deck.reserve(deckSize);
  for (size_t i{}; i < static_cast<size_t>(CardValue::Count); i++) {
    for (size_t j{}; j < static_cast<size_t>(CardType::Count); j++) {
      auto value = static_cast<CardValue>(i);
      auto type = static_cast<CardType>(j);
      deck.emplace_back(*moveManager_, value, type,
                        "test " + std::to_string(i) + " " + std::to_string(j));
    }
  }

  std::random_device rd;
  std::mt19937 gen{rd()};

  // std::shuffle(deck.begin(), deck.end(), gen);

  return deck;
}

std::expected<CardPosition, Error> Board::search(const CardCode &code) const {
  auto tableauPos = tableau_->search(code);
  if (tableauPos)
    return tableauPos.value();

  return std::unexpected(ErrorCardPositionNotFound(code).error());
}

Tableau &Board::tableau() const { return *tableau_; }

ReserveStack &Board::reserveStack() const { return *reserveStack_; }

} // namespace solitairecpp
