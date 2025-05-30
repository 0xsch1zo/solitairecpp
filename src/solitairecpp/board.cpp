#include <algorithm>
#include <ftxui/component/component.hpp>
#include <memory>
#include <print>
#include <random>
#include <ranges>
#include <solitairecpp/board.hpp>
#include <solitairecpp/cards.hpp>
#include <solitairecpp/move_manager.hpp>
#include <thread>

namespace solitairecpp {

Tableau::Tableau(StartCards cards, MoveManager &moveManager)
    : moveManager_{moveManager} {
  std::vector cardsVec(cards.begin(), cards.end()); // it's just easier to copy

  size_t rowSize = 1;
  for (auto &cardRow : tableau_) {
    std::vector cardRowCards(cardsVec.begin(), cardsVec.begin() + rowSize);
    cardRowCards.back().show(); // The last card is visible
    cardRow.illegalAppend(cardRowCards);
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
Tableau::illegalAppendTo(const AppendCardPosition &pos, const Cards &cards) {
  if (pos.cardRowIndex >= tableau_.size())
    return std::unexpected(ErrorInvalidCardIndex().error());

  tableau_.at(pos.cardRowIndex).illegalAppend(cards);
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

ReserveStack::ReserveStack(Difficulty mode, StartCards cards)
    : mode_{mode}, viewableCardsComponent_{ft::Container::Horizontal({})} {
  for (auto &card : cards)
    card.show();

  hiddenCards_.reserve(cards.size());
  hiddenCards_.append_range(cards);
}

void ReserveStack::moveToHiddenAndShuffle() {
  hiddenCards_.append_range(viewedCards_);
  viewedCards_.erase(viewedCards_.begin(), viewedCards_.end());

  std::random_device rd;
  std::mt19937 gen{rd()};
  std::shuffle(hiddenCards_.begin(), hiddenCards_.end(), gen);
}

void ReserveStack::revealEasy() {
  viewedCards_.emplace_back(hiddenCards_.front());
  hiddenCards_.erase(hiddenCards_.begin());

  if (viewableCardsComponent_->ChildCount() > 0)
    viewableCardsComponent_->ChildAt(0)->Detach();
  viewableCardsComponent_->Add(viewedCards_.back().component());
}

void ReserveStack::revealHard() {
  if (hiddenCards_.size() < hardDifficultyViewableAmount) {
    viewedCards_.insert(viewedCards_.end(), hiddenCards_.begin(),
                        hiddenCards_.end());

    hiddenCards_.erase(hiddenCards_.begin(), hiddenCards_.end());
  } else {
    viewedCards_.insert(viewedCards_.end(), hiddenCards_.begin(),
                        hiddenCards_.begin() + hardDifficultyViewableAmount);

    hiddenCards_.erase(hiddenCards_.begin(),
                       hiddenCards_.begin() + hardDifficultyViewableAmount);
  }

  viewableCardsComponent_->DetachAllChildren();
  viewableCardsComponent_->Add(viewedCards_.back().component());
}

void ReserveStack::reveal() {
  if (hiddenCards_.size() == 0 && viewedCards_.size() != 0)
    moveToHiddenAndShuffle();
  else if (hiddenCards_.size() == 0)
    return; // we ran out of cards

  switch (mode_) {
  case Difficulty::Easy:
    revealEasy();
    break;
  case Difficulty::Hard:
    revealHard();
    break;
  }
}

ft::Component ReserveStack::component() {
  return ft::Container::Horizontal(
      {ft::Button({.on_click = [&] { reveal(); },
                   .transform =
                       [&](const ft::EntryState state) {
                         ft::Element element;
                         if (hiddenCards_.size() == 0 &&
                             viewedCards_.size() <= 1)
                           element = ft::text("No more cards in reserve");
                         else if (hiddenCards_.size() == 0)
                           element = ft::text("shuffle");
                         else
                           element = ft::text("reserve stack - hidden");
                         element |= ft::border;

                         if (state.active)
                           element |= ft::bold;
                         if (state.focused)
                           element |= ft::inverted;
                         return element;
                       }}),
       viewableCardsComponent_});
}

std::expected<ReserveStack::CardPosition, Error>
ReserveStack::searchViewable(const CardCode &code) {
  for (size_t i{1};
       i <= std::min(hardDifficultyViewableAmount, viewedCards_.size()); i++) {
    if (viewedCards_.at(viewedCards_.size() - i).code() == code)
      return CardPosition{};
  }

  return std::unexpected(ErrorCardPositionNotFound(code).error());
}

std::expected<Card, Error> ReserveStack::getTopCard() {
  if (viewedCards_.size() < 0)
    return std::unexpected(ErrorInvalidCardIndex().error());
  return viewedCards_.back();
}

std::expected<void, Error> ReserveStack::deleteTopCard() {
  if (viewableCardsComponent_->ChildCount() == 0 || viewedCards_.size() == 0)
    return std::unexpected(ErrorInvalidCardIndex().error());

  viewableCardsComponent_->DetachAllChildren();
  viewedCards_.erase(viewedCards_.end() - 1);
  return std::expected<void, Error>();
}

std::expected<void, Error> ReserveStack::setTopCard(const Card &card) {
  viewedCards_.emplace_back(card);

  if (mode_ == Difficulty::Easy && viewableCardsComponent_->ChildCount() == 1)
    return std::unexpected(ErrorIllegalMove().error());

  if (mode_ == Difficulty::Hard &&
      viewableCardsComponent_->ChildCount() == hardDifficultyViewableAmount)
    return std::unexpected(ErrorIllegalMove().error());

  viewableCardsComponent_->Add(card.component());
  return std::expected<void, Error>();
}

Foundations::Foundations(MoveManager &moveManager)
    : moveManager_{moveManager}, component_{ft::Container::Horizontal({})} {
  for (size_t i{}; i < foundations_.size(); i++)
    component_->Add(placeholder(i));
}

// empty card field. contiainer vertical is used because the lib doesn't
// offer a way to insert components at an index from what I know
ft::Component Foundations::placeholder(size_t index) {
  return ft::Container::Vertical(
      {ft::Button({.on_click =
                       [=, *this] {
                         std::thread([=, *this] {
                           moveManager_.setMoveTarget(
                               CardPosition{.foundationIndex = index});
                         }).detach();
                       },
                   .transform =
                       [=, *this](const ft::EntryState state) {
                         auto element = ft::text("");
                         element |= Card::cardWidth | Card::cardHeight;
                         element |= ft::border;

                         if (state.active)
                           element |= ft::bold;
                         if (state.focused)
                           element |= ft::inverted;
                         if (moveManager_.moveTransactionOpen())
                           element |= ft::color(ft::Color::Green);

                         return element;
                       }})});
}

std::expected<void, Error> Foundations::set(const CardPosition &pos,
                                            const Card &card) {
  if (pos.foundationIndex >= foundations_.size() ||
      pos.foundationIndex >= component_->ChildCount())
    return std::unexpected(ErrorInvalidCardIndex().error());

  foundations_.at(pos.foundationIndex) = card;
  component_->ChildAt(pos.foundationIndex)
      ->DetachAllChildren(); // hack to get insertion working as previously
                             // mentionedd
  component_->ChildAt(pos.foundationIndex)->Add(card.component());
  return std::expected<void, Error>();
}

ft::Component Foundations::component() { return component_; }

std::expected<Foundations::CardPosition, Error>
Foundations::search(const CardCode &code) {
  for (const auto [i, foundation] :
       std::views::zip(std::views::iota(0UL), foundations_)) {
    if (!foundation.has_value())
      continue;

    if (foundation->code() == code)
      return CardPosition{.foundationIndex = i};
  }

  return std::unexpected(ErrorCardPositionNotFound(code).error());
}

Board::Board() : moveManager_{std::make_unique<MoveManager>(*this)} {
  Cards deck = buildDeck();

  Tableau::StartCards tabelauCards =
      [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        return Tableau::StartCards{{deck.at(Is)...}};
      }(std::make_index_sequence<
          Tableau::startCardsSize>{}); // This is just initializing the array
                                       // with elements from the deck. Yes
                                       // there is problably no other simpler
                                       // way.
  tableau_ = std::make_unique<Tableau>(tabelauCards, *moveManager_);
  deck.erase(deck.begin(), deck.begin() + Tableau::startCardsSize);

  ReserveStack::StartCards reserveStackCards =
      [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        return ReserveStack::StartCards{{deck.at(Is)...}};
      }(std::make_index_sequence<ReserveStack::startCardsSize>{});
  reserveStack_ =
      std::make_unique<ReserveStack>(Difficulty::Easy, reserveStackCards);
}

ft::Component Board::component() const {
  return ft::Container::Horizontal({foundations_->component(),
                                    tableau_->component(),
                                    reserveStack_->component()});
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

  auto reserveStackPos = reserveStack_->searchViewable(code);
  if (reserveStackPos)
    return reserveStackPos.value();

  auto foundationsStackPos = foundations_->search(code);
  if (foundationsStackPos)
    return foundationsStackPos.value();

  return std::unexpected(ErrorCardPositionNotFound(code).error());
}

Tableau &Board::tableau() const { return *tableau_; }

ReserveStack &Board::reserveStack() const { return *reserveStack_; }

Foundations &Board::foundations() const { return *foundations_; }

} // namespace solitairecpp
