#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <print>
#include <solitairecpp/cards.hpp>
#include <solitairecpp/error.hpp>
#include <solitairecpp/move_manager.hpp>
#include <stdexcept>

namespace solitairecpp {

bool CardCode::operator==(const CardCode &rhs) const {
  return value == rhs.value && type == rhs.type;
}

std::string CardSerializer::Encode(const CardCode &cardCode) {
  // Of course very advanced encoding is being used :)
  // Think it's enogh for these purposes, using json seems to overkill
  return std::format("{}{}{}{}", magic_, static_cast<int>(cardCode.value),
                     delimeter_, static_cast<int>(cardCode.type));
}

std::expected<CardCode, Error>
CardSerializer::Decode(const std::string &serializedCard) {
  if (!serializedCard.starts_with(magic_))
    return std::unexpected(ErrorParser(serializedCard).error());

  auto serializedCardPreParsed = serializedCard.substr(magic_.size());
  auto delimeter = serializedCardPreParsed.find(delimeter_);
  if (delimeter == std::string::npos ||
      delimeter == serializedCardPreParsed.size() - 1) // Can't be the last item
    return std::unexpected(ErrorParser(serializedCard).error());

  int cardValue{}, cardType{};
  try {
    cardValue = std::stoi(serializedCardPreParsed.substr(0, delimeter));
    cardType = std::stoi(serializedCardPreParsed.substr(delimeter + 1));
  } catch (std::invalid_argument
               &e) { // Yes it could be handled with just std::excpetion but I
                     // think it's better in terms of clarity
    return std::unexpected(ErrorParser(serializedCard).error());
  } catch (std::out_of_range &e) {
    return std::unexpected(ErrorParser(serializedCard).error());
  }

  if (cardValue < 0 || cardValue >= static_cast<int>(CardValue::Count))
    return std::unexpected(ErrorParser(serializedCard).error());
  if (cardType < 0 || cardType >= static_cast<int>(CardType::Count))
    return std::unexpected(ErrorParser(serializedCard).error());

  return CardCode{.value = static_cast<CardValue>(cardValue),
                  .type = static_cast<CardType>(cardType)};
}

Card::Card(const MoveManager &moveManager, CardValue value, CardType type,
           std::string art, bool hidden)
    : moveManager_{moveManager}, value_{value}, type_{type}, art_{art},
      hidden_{hidden} {
  component_ = ft::Button(
      {.on_click =
           [=, *this] {
             auto *screen = ft::ScreenInteractive::Active();
             if (screen == nullptr)
               throw std::runtime_error(
                   "Couldn't get handle on current screen"); // Cooked

             screen->PostEvent(ft::Event::Special(
                 CardSerializer::Encode({.value = value_, .type = type_})));
           },
       .transform =
           [=, *this](const ft::EntryState state) {
             auto element = ft::text(art_);
             element |= cardWidth | cardHeight;
             element |= ft::border;
             if (moveManager_.isBeingMoved(code()))
               element |= ft::color(ft::Color::Green);

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
  return std::expected<void, Error>();
}

ft::Component CardRow::component() const {
  auto moveTargetBar = ft::Button(
      {.on_click =
           [*this] {
             moveManager_.setMoveTarget(
                 Tableau::AppendCardPosition{.cardRowIndex = index_});
           },
       .transform =
           [*this](const ft::EntryState &state) {
             auto element = ft::separator();

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
