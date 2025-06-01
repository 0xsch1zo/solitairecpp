#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <random>
#include <solitairecpp/board.hpp>
#include <solitairecpp/move_manager.hpp>
#include <thread>

namespace solitairecpp {

ReserveStack::ReserveStack(Difficulty mode, MoveManager &moveManager,
                           StartCards cards)
    : mode_{mode}, viewableCardsComponent_{ft::Container::Horizontal({})},
      moveManager_{moveManager} {
  for (auto &card : cards)
    card.show();

  hiddenCards_.reserve(cards.size());
  hiddenCards_.append_range(cards);
}

ReserveStack::ReserveStackCard::ReserveStackCard(
    const Card &card, const ReserveStack &reserveStack)
    : Card(card), reserveStack_{reserveStack} {
  std::shared_ptr<int> index = std::make_shared<int>();
  component_ = ft::Button(
      {.on_click =
           [=, *this] {
             if (*index !=
                 reserveStack_.viewableCardsComponent_->ChildCount() - 1)
               return;
             std::thread([*this] {
               moveManager_.cardSelected(code());
             }).detach();
           },
       .transform =
           [=, *this](const ft::EntryState state) {
             *index = state.index;
             auto element = ft::text(art_) | ft::center;
             element |= cardWidth | cardHeight;
             element |= ft::border;
             switch (color_) {
             case CardColor::Red:
               element |= ft::color(ft::Color::Red);
               break;
             case CardColor::Black:
               element |= ft::color(ft::Color::GrayDark);
               break;
             }

             if (state.index !=
                 reserveStack_.viewableCardsComponent_->ChildCount() - 1)
               return element;

             if (state.active)
               element |= ft::bold;
             if (state.focused)
               element |= ft::inverted;
             return element;
           }});
}

ft::Component ReserveStack::ReserveStackCard::component() const {
  return component_;
}

void ReserveStack::moveToHiddenAndShuffle() {
  hiddenCards_.append_range(viewedCards_);
  viewedCards_.erase(viewedCards_.begin(), viewedCards_.end());

  std::random_device rd;
  std::mt19937 gen{rd()};
  std::shuffle(hiddenCards_.begin(), hiddenCards_.end(), gen);
}

void ReserveStack::revealEasy() {
  viewedCards_.emplace_back(hiddenCards_.back());
  hiddenCards_.erase(hiddenCards_.end() - 1);

  if (viewableCardsComponent_->ChildCount() > 0)
    viewableCardsComponent_->ChildAt(0)->Detach();
  viewableCardsComponent_->Add(viewedCards_.back().component());
}

// fix this
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
  for (size_t i{std::min(hardDifficultyViewableAmount, viewedCards_.size())};
       i > 0 && !viewedCards_.empty(); i--) {
    ReserveStackCard reserveStackCard(viewedCards_.at(viewedCards_.size() - i),
                                      *this);
    viewableCardsComponent_->Add(reserveStackCard.component());
  }
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

  moveManager_.setMoveOrigin(CardPosition{});
  moveManager_.setMoveTarget(CardPosition{});
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
  if (viewedCards_.empty())
    return std::unexpected(ErrorInvalidCardIndex().error());
  return viewedCards_.back();
}

std::expected<void, Error> ReserveStack::deleteTopCard() {
  if (viewableCardsComponent_->ChildCount() == 0 || viewedCards_.size() == 0)
    return std::unexpected(ErrorInvalidCardIndex().error());

  viewableCardsComponent_->ChildAt(viewableCardsComponent_->ChildCount() - 1)
      ->Detach();
  viewedCards_.erase(viewedCards_.end() - 1);

  return std::expected<void, Error>();
}

std::expected<void, Error> ReserveStack::setTopCard(const Card &card) {
  switch (mode_) {
  case Difficulty::Easy:
    if (viewableCardsComponent_->ChildCount() == 1)
      return std::unexpected(ErrorIllegalMove().error());
    break;
  case Difficulty::Hard:
    if (viewableCardsComponent_->ChildCount() == hardDifficultyViewableAmount)
      return std::unexpected(ErrorIllegalMove().error());
    break;
  }
  viewedCards_.emplace_back(card);
  viewableCardsComponent_->Add(card.component());
  return std::expected<void, Error>();
}

std::expected<void, Error> ReserveStack::rollbackCard() {
  if (viewedCards_.empty())
    return std::unexpected(ErrorInvalidCardIndex().error());
  if (mode_ == Difficulty::Easy) {
    hiddenCards_.emplace_back(viewedCards_.back());

    auto deleteSuccess =
        deleteTopCard(); // it's supposed to be used for somehting else, but why
                         // don't reuse it
    if (!deleteSuccess)
      return std::unexpected(deleteSuccess.error());

    if (!viewedCards_.empty())
      viewableCardsComponent_->Add(viewedCards_.back().component());
  } else {
    hiddenCards_.insert(
        hiddenCards_.begin(),
        viewedCards_.end() -
            std::min<size_t>(viewedCards_.size(), hardDifficultyViewableAmount),
        viewedCards_.end());

    viewableCardsComponent_->DetachAllChildren();
    viewedCards_.erase(
        viewedCards_.end() -
            std::min(viewedCards_.size(), hardDifficultyViewableAmount),
        viewedCards_.end());

    for (size_t i{std::min(hardDifficultyViewableAmount, viewedCards_.size())};
         i > 0 && !viewedCards_.empty(); i--) {
      ReserveStackCard reserveStackCard(
          viewedCards_.at(viewedCards_.size() - i), *this);
      viewableCardsComponent_->Add(reserveStackCard.component());
    }
  }

  return std::expected<void, Error>();
}

} // namespace solitairecpp
