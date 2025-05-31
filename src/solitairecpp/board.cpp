#include <algorithm>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <memory>
#include <print>
#include <random>
#include <ranges>
#include <solitairecpp/board.hpp>
#include <solitairecpp/cards.hpp>
#include <solitairecpp/move_manager.hpp>
#include <solitairecpp/utils.hpp>
#include <stdexcept>
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
                         std::string label;
                         if (hiddenCards_.size() == 0 &&
                             viewedCards_.size() <= 1)
                           label = "No more cards in reserve";
                         else if (hiddenCards_.size() == 0)
                           label = "shuffle";
                         else
                           label = "reserve stack";
                         auto element = ft::text(label) | ft::center;
                         element |= Card::cardWidth | Card::cardHeight;
                         element |= ft::border;

                         if (state.active)
                           element |= ft::bold;
                         if (state.focused)
                           element |= ft::inverted;
                         return element;
                       }}),
       ft::Renderer(viewableCardsComponent_, [*this] {
         if (viewableCardsComponent_->ChildCount() == 0)
           return ft::text("") | Card::cardWidth | Card::cardHeight |
                  ft::border;
         else
           return viewableCardsComponent_->Render();
       })});
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

Foundations::Foundations(MoveManager &moveManager,
                         std::function<void()> onGameWon)
    : moveManager_{moveManager}, component_{ft::Container::Horizontal({})},
      onGameWon_{onGameWon} {
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

  if (!isSetLegal(pos, card))
    return std::unexpected(ErrorIllegalMove().error());

  foundations_.at(pos.foundationIndex).emplace_back(card);
  component_->ChildAt(pos.foundationIndex)
      ->DetachAllChildren(); // hack to get insertion working as previously
                             // mentionedd
  component_->ChildAt(pos.foundationIndex)
      ->Add(FoundationCard(card).component());

  for (const auto &foundation : foundations_) {
    if (!foundation.empty() &&
        foundation.back().code().value != CardValue::King)
      return std::expected<void, Error>();
  }
  onGameWon_();
  return std::expected<void, Error>();
}

ft::Component Foundations::component() { return component_; }

std::expected<Foundations::CardPosition, Error>
Foundations::search(const CardCode &code) {
  for (const auto [i, foundation] :
       std::views::zip(std::views::iota(0UL), foundations_)) {
    if (foundation.empty())
      continue;

    if (foundation.back().code() == code)
      return CardPosition{.foundationIndex = i};
  }

  return std::unexpected(ErrorCardPositionNotFound(code).error());
}

std::expected<bool, Error> Foundations::isSetLegal(const CardPosition &pos,
                                                   const Card &card) {
  if (pos.foundationIndex >= foundations_.size())
    return std::unexpected(ErrorInvalidCardIndex().error());

  const auto &foundation = foundations_.at(pos.foundationIndex);
  if (foundation.empty())
    return card.code().value == CardValue::Ace;

  return static_cast<int>(card.code().value) ==
             static_cast<int>(foundation.back().code().value) + 1 &&
         card.code().type == foundation.back().code().type;
}

ft::Component ExitButton::component() {
  return ft::Button({
      .label = "Exit Game",
      .on_click =
          [] {
            auto *screen = ft::ScreenInteractive::Active();
            if (screen == nullptr)
              throw std::runtime_error("Couldn't get active screen to exit");
            screen->Exit();
          },
  });
}

Board::Board(Difficulty mode, std::function<void()> onGameWon)
    : moveManager_{std::make_unique<MoveManager>(*this)} {
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
  reserveStack_ = std::make_unique<ReserveStack>(mode, reserveStackCards);
}

ft::Component Board::component() const {
  auto sidepanel = ft::Container::Vertical({foundations_->component(),
                                            reserveStack_->component(),
                                            ExitButton::component()});
  auto tableau = tableau_->component();

  auto board = ft::Container::Horizontal({sidepanel, tableau});
  return ft::Container::Horizontal({ft::Renderer(board, [=] {
    return ft::hbox(ft::vbox(sidepanel->ChildAt(0)->Render(), ft::separator(),
                             sidepanel->ChildAt(1)->Render(), ft::separator(),
                             ft::filler(), sidepanel->ChildAt(2)->Render()),
                    ft::separator(), ft::filler(), tableau->Render());
  })});
}

Cards Board::buildDeck() {
  const auto deckSize = 52;
  Cards deck;
  ArtGenerator generator;
  deck.reserve(deckSize);
  for (size_t i{}; i < static_cast<size_t>(CardValue::Count); i++) {
    for (size_t j{}; j < static_cast<size_t>(CardType::Count); j++) {
      auto value = static_cast<CardValue>(i);
      auto type = static_cast<CardType>(j);
      deck.emplace_back(*moveManager_, value, type,
                        generator.generate(value, type));
    }
  }

  std::random_device rd;
  std::mt19937 gen{rd()};

  std::shuffle(deck.begin(), deck.end(), gen);

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
