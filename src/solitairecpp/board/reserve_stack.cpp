#include <random>
#include <solitairecpp/board.hpp>
#include <solitairecpp/move_manager.hpp>

namespace solitairecpp {

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
  if (viewedCards_.empty())
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
  switch (mode_) {
  case Difficulty::Easy:
    if (!viewedCards_.empty() || viewableCardsComponent_->ChildCount() == 1)
      return std::unexpected(ErrorIllegalMove().error());
    break;
  case Difficulty::Hard:
    if (viewedCards_.size() == hardDifficultyViewableAmount ||
        viewableCardsComponent_->ChildCount() == hardDifficultyViewableAmount)
      return std::unexpected(ErrorIllegalMove().error());
    break;
  }
  viewedCards_.emplace_back(card);
  viewableCardsComponent_->Add(card.component());
  return std::expected<void, Error>();
}

} // namespace solitairecpp
