#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <print>
#include <solitairecpp/cards.hpp>
#include <solitairecpp/error.hpp>
#include <solitairecpp/move_manager.hpp>

namespace solitairecpp {

bool CardCode::operator==(const CardCode &rhs) const {
  return value == rhs.value && type == rhs.type;
}

Card::Card(MoveManager &moveManager, CardValue value, CardType type,
           std::string art, bool hidden)
    : moveManager_{moveManager}, value_{value}, type_{type}, art_{art},
      hidden_{std::make_shared<bool>(hidden)} {
  component_ =
      ft::Button({.on_click =
                      [=, *this] {
                        if (*hidden_)
                          return;

                        std::thread([*this] {
                          moveManager_.cardSelected(code());
                        }).detach();
                      },
                  .transform =
                      [=, *this](const ft::EntryState state) {
                        auto element = ft::text(*hidden_ ? "hidden" : art_);
                        element |= cardWidth | cardHeight;
                        element |= ft::border;
                        if (moveManager_.isBeingMoved(code()))
                          element |= ft::color(ft::Color::Green);

                        // shoud not focus if the transaction is open or the
                        // card is hidden
                        if (moveManager_.moveTransactionOpen() || *hidden_)
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

Card &Card::operator=(const Card &other) {
  if (this == &other)
    return *this;

  value_ = other.value_;
  type_ = other.type_;
  art_ = other.art_;
  hidden_ = other.hidden_;
  component_ = other.component_;
  return *this;
}

void Card::show() { *hidden_ = false; }

ft::Component Card::component() const { return std::move(component_); }

CardCode Card::code() const { return {.value = value_, .type = type_}; }

CardRow::CardRow(size_t index, MoveManager &moveManager)
    : cardsComponent_{ft::Container::Vertical({})}, moveManager_{moveManager},
      index_{index} {}

std::expected<void, Error> CardRow::append(const Cards &cards) {
  // TODO: Check if card range is valid
  cards_.reserve(cards.size());
  for (const auto &card : cards) {
    cards_.emplace_back(card);
    cardsComponent_->Add(card.component());
  }
  return std::expected<void, Error>();
}

std::expected<void, Error> CardRow::deleteFrom(const CardPosition &pos) {
  if (pos.cardIndex >= cards_.size())
    return std::unexpected(ErrorInvalidCardIndex().error());

  for (size_t i{pos.cardIndex}; i < cards_.size(); i++)
    cardsComponent_->ChildAt(pos.cardIndex)->Detach();

  cards_.erase(cards_.begin() + pos.cardIndex, cards_.end());

  if (cards_.size() != 0)
    cards_.back().show(); // Reveal the last card
  return std::expected<void, Error>();
}

ft::Component CardRow::component() const {
  auto moveTargetBar = ft::Button(
      {.on_click =
           [*this] {
             if (!moveManager_.moveTransactionOpen())
               return;
             moveManager_.setMoveTarget(
                 Tableau::AppendCardPosition{.cardRowIndex = index_});
           },
       .transform =
           [*this](const ft::EntryState &state) {
             auto element = ft::separator();

             // should not focus if transaction is not open
             if (!moveManager_.moveTransactionOpen())
               return element;

             if (state.active)
               element |= ft::bold;
             if (state.focused)
               element |= ft::inverted;

             if (moveManager_.isTargetError(
                     Tableau::AppendCardPosition{.cardRowIndex = index_}))
               element |= ft::color(ft::Color::Red);

             return element;
           }});
  return std::move(ft::Container::Vertical({cardsComponent_, moveTargetBar}));
}

std::expected<CardRow::CardPosition, Error>
CardRow::search(const CardCode &code) const {
  for (const auto [i, card] :
       std::views::zip(std::views::iota(0ULL), cards_)) { // C++23 baby
    if (code == card.code())
      return CardPosition{.cardIndex = i};
  }

  return std::unexpected(ErrorCardPositionNotFound(code).error());
}

std::expected<Cards, Error> CardRow::getCardsFrom(const CardPosition &pos) {
  if (pos.cardIndex >= cards_.size())
    return std::unexpected(ErrorInvalidCardIndex().error());

  Cards res(cards_.begin() + pos.cardIndex, cards_.end());
  return res;
}

} // namespace solitairecpp
