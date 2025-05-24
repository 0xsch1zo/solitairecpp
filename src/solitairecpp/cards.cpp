#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <print>
#include <solitairecpp/cards.hpp>
#include <solitairecpp/error.hpp>
#include <stdexcept>
#include <vector>

namespace solitairecpp {

std::string CardSerializer::Encode(const CardCode &cardCode) {
  // Of course very advanced encoding is being used :)
  // Think it's enogh for these purposes, using json seems to overkill
  return std::format("{}{}-{}", magic_, static_cast<int>(cardCode.value),
                     static_cast<int>(cardCode.type));
}

std::expected<CardCode, Error>
CardSerializer::Decode(const std::string &serializedCard) {
  if (!serializedCard.starts_with(magic_))
    return std::unexpected(ErrorParser(serializedCard).error());

  auto serializedCardPreParsed = serializedCard.substr(magic_.size() - 1);
  auto delimeter = serializedCardPreParsed.find('-');
  if (delimeter == std::string::npos)
    return std::unexpected(ErrorParser(serializedCard).error());

  int cardValue{}, cardType{};
  try {
    cardValue = std::stoi(serializedCardPreParsed.substr(0, delimeter));
    cardType = std::stoi(serializedCardPreParsed.substr(delimeter));
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
    : value_{value}, type_{type}, art_{art}, hidden_{hidden} {}

std::expected<void, Error> CardRow::appendCards(Cards::iterator begin,
                                                Cards::iterator end) {
  // TODO: Check if card range is valid
  cards_.insert(cards_.end(), begin, end);
  return std::expected<void, Error>();
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
}

std::expected<Cards, Error> CardRow::getCardsFrom(size_t cardIndex) const {
  if (cardIndex >= cards_.size())
    return std::unexpected(ErrorInvalidCardIndex().error());

  Cards result(cards_.begin() + cardIndex, cards_.end());
  return result;
}

ft::Component CardRow::component() const {
  auto container = ft::Container::Vertical({});
  for (const auto &card : cards_)
    container->Add(card.component());
  return container;
}

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

} // namespace solitairecpp
