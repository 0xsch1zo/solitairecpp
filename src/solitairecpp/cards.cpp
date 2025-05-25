#include <algorithm>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <iterator>
#include <print>
#include <solitairecpp/cards.hpp>
#include <solitairecpp/error.hpp>
#include <stdexcept>
#include <vector>

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

Card::Card(CardValue value, CardType type, std::string art, bool hidden)
    : value_{value}, type_{type}, art_{art}, hidden_{hidden} {
  /*component_ = ft::Button(
      {.on_click =
           [&] {
             auto *screen = ft::ScreenInteractive::Active();
             if (screen == nullptr)
               throw std::runtime_error(
                   "Couldn't get handle on current screen"); // Cooked

             screen->PostEvent(ft::Event::Special(
                 CardSerializer::Encode({.value = value_, .type = type_})));
           },
       .transform =
           [&](const ft::EntryState state) {
             auto element = ft::text(art_);
             element |= cardWidth | cardHeight;
             element |= ft::border;

             if (state.active) {
               element |= ft::bold;
             }
             if (state.focused) {
               element |= ft::inverted;
             }
             return element;
           }});*/
}

ft::Component Card::component() const {
  return ft::Button(
      {.on_click =
           [&] {
             auto *screen = ft::ScreenInteractive::Active();
             if (screen == nullptr)
               throw std::runtime_error(
                   "Couldn't get handle on current screen"); // Cooked

             screen->PostEvent(ft::Event::Special(
                 CardSerializer::Encode({.value = value_, .type = type_})));
           },
       .transform =
           [&](const ft::EntryState state) {
             auto element = ft::text(art_);
             element |= cardWidth | cardHeight;
             element |= ft::border;

             if (state.active) {
               element |= ft::bold;
             }
             if (state.focused) {
               element |= ft::inverted;
             }
             return element;
           }});
  ;
}

Card::~Card() { /*component_->Detach();*/ }

CardCode Card::code() const { return {.value = value_, .type = type_}; }

std::expected<void, Error> CardRow::append(const Cards &cards) {
  // TODO: Check if card range is valid
  // cards_.insert_range(cards_.begin() + pos.cardIndex, cards);
  // std::for_each(begin, end,
  //             [&](const auto &card) { cards_.emplace_back(card); });
  for (const auto &card : cards) {
    cards_.push_back(card);
    component_->Add(card.component());
  }
  return std::expected<void, Error>();
}

std::expected<void, Error> CardRow::deleteFrom(const CardPosition &pos) {
  if (pos.cardIndex >= cards_.size())
    return std::unexpected(ErrorInvalidCardIndex().error());

  cards_.erase(cards_.begin() + pos.cardIndex, cards_.end());
  return std::expected<void, Error>();
}

CardRow::CardRow() : component_{ft::Container::Vertical({})} {}

// fix this shit
ft::Component CardRow::component() const {
  /*auto container = ft::Container::Vertical({});
  for (const auto &card : cards_)
    container->Add(card.component());
  return container;*/
  return component_;
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

} // namespace solitairecpp
