#pragma once

#include <expected>
#include <string>
#include <vector>

#include <solitairecpp/solitairecpp.hpp>

namespace solitairecpp {

class ErrorInvaidCardRange : Error {
  std::string what() { return "The card range supplied has the wrong color"; }
};

class ErrorInvalidCardIndex : Error {
  std::string what() {
    return "Tried to access a card at a non-existsent index";
  }
};

enum class CardValue {
  Ace,
  Two,
  Three,
  Six,
  Seven,
  Eight,
  Nine,
  Ten,
  Jack,
  Queen,
  King
};

enum class CardType { Hearts, Diamonds, Spades, Clubs };

class Card {
  explicit Card(CardValue value, CardType type, std::string art);

private:
  CardValue value_;
  CardType type_;
  std::string art_;
};

typedef std::vector<Card> Cards;

// wrapper around std::vector
class CardRow {
public:
  CardRow() = default;
  std::expected<void, Error> appendCards(const Cards &cards);
  std::expected<Cards, Error> getCardsFrom(size_t cardIndex);

private:
  Cards cards_;
};

} // namespace solitairecpp
