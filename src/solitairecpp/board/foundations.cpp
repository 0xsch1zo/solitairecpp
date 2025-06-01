#include <ranges>
#include <solitairecpp/board.hpp>
#include <solitairecpp/move_manager.hpp>
#include <thread>

namespace solitairecpp {

Foundations::Foundations(MoveManager &moveManager,
                         std::function<void()> onGameWon)
    : moveManager_{moveManager}, component_{ft::Container::Horizontal({})},
      onGameWon_{onGameWon} {
  for (size_t i{}; i < foundations_.size(); i++)
    component_->Add(placeholder(i));
}

Foundations::FoundationCard::FoundationCard(const Card &card) : Card(card) {
  component_ = ft::Button({.on_click =
                               [=, *this] {
                                 std::thread([*this] {
                                   moveManager_.setMoveTarget(code());
                                 }).detach();
                               },
                           .transform =
                               [=, *this](const ft::EntryState state) {
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

                                 if (!moveManager_.moveTransactionOpen())
                                   return element;

                                 if (state.active) {
                                   element |= ft::bold;
                                 }
                                 if (state.focused) {
                                   element |= ft::inverted;
                                 }
                                 return element;
                               }});
}

ft::Component Foundations::FoundationCard::component() const {
  return component_;
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

                         // aware of what it seems like repetition, it's needed
                         if (!moveManager_.moveTransactionOpen())
                           return element;

                         if (state.active)
                           element |= ft::bold;
                         if (state.focused)
                           element |= ft::inverted;

                         if (moveManager_.moveTransactionOpen())
                           element |= ft::color(ft::Color::Green);

                         return element;
                       }})});
}

std::expected<Card, Error> Foundations::acquireCard(const CardPosition &pos) {
  if (pos.foundationIndex >= foundations_.size() ||
      pos.foundationIndex >= component_->ChildCount())
    return std::unexpected(ErrorInvalidCardIndex().error());
  auto &foundation = foundations_.at(pos.foundationIndex);

  if (foundation.empty())
    return std::unexpected(ErrorInvalidCardIndex().error());

  auto card = foundation.back();
  foundation.erase(foundation.end() - 1);
  component_->ChildAt(pos.foundationIndex)->DetachAllChildren();
  component_->ChildAt(pos.foundationIndex)
      ->Add(placeholder(pos.foundationIndex));
  return card;
}

std::expected<void, Error> Foundations::setCard(const CardPosition &pos,
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

} // namespace solitairecpp
