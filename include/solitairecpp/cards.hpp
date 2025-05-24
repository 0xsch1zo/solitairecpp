#pragma once

#include <expected>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <solitairecpp/error.hpp>
#include <string>
#include <vector>

namespace ft = ftxui;

namespace solitairecpp {

class ErrorInvalidCardRange : public ErrorBase {
public:
  std::string what() override {
    return "The card range supplied has the wrong color";
  }

  Error error() override { return std::make_shared<ErrorInvalidCardRange>(); }
};

class ErrorInvalidCardIndex : public ErrorBase {
public:
  std::string what() override {
    return "Tried to access a card at a non-existsent index";
  }

  Error error() override { return std::make_shared<ErrorInvalidCardIndex>(); }
};

class ErrorParser : public ErrorBase {
public:
  ErrorParser(const std::string &invalidString)
      : invalidString_{invalidString} {}
  std::string what() override {
    return std::format("Invalid string was passed to the parser: {}",
                       invalidString_);
  }

  Error error() override {
    return std::make_shared<ErrorParser>(invalidString_);
  }

private:
  std::string invalidString_;
};

enum class CardValue {
  Ace,
  Two,
  Three,
  Four,
  Five,
  Six,
  Seven,
  Eight,
  Nine,
  Ten,
  Jack,
  Queen,
  King,
  Count // Need to itarate over this enum
};

enum class CardType { Hearts, Diamonds, Spades, Clubs, Count };

// Due to each card in the deck being unique we can idenitfy them by thier value
// combined with thier type/color. Probably the best method in terms of
// complexity.
struct CardCode {
  CardValue value;
  CardType type;
};

class CardSerializer {
public:
  static std::string Encode(const CardCode &cardCode);

  static std::expected<CardCode, Error>
  Decode(const std::string &serializedCard);

private:
  static constexpr std::string magic_ = "CardCode: ";
};

class Card {
public:
  Card() = default;
  Card(CardValue value, CardType type, std::string art, bool hidden = true);
  void show();
  ft::Component component() const;

  // Tempororary remove at release
  std::string getArt() const { return art_; }

private:
  void PostCardEvent();

private:
  static inline const auto cardWidth = ft::size(ft::WIDTH, ft::EQUAL, 20);
  static inline const auto cardHeight = ft::size(ft::WIDTH, ft::EQUAL, 7);
  bool hidden_;
  CardValue value_;
  CardType type_;
  std::string art_ = "art not initalized";
};

typedef std::vector<Card> Cards;

// wrapper around std::vector
class CardRow {
public:
  CardRow() = default;
  ft::Component component() const;
  std::expected<void, Error> appendCards(Cards::iterator begin,
                                         Cards::iterator end);
  std::expected<Cards, Error> getCardsFrom(size_t cardIndex) const;

private:
  Cards cards_;
};

typedef std::array<Card, 28>
    StartTableauCards; // The main table has 28 cards at the start

class Tableau {
public:
  Tableau(StartTableauCards &&cards);
  ft::Component component() const;

private:
  std::array<CardRow, 7> tableau_;
};

typedef std::array<Card, 24>
    StartReserveStackCards; // The reserve stack has 24 cards at the start

class ReserveStack {
public:
  ReserveStack(StartReserveStackCards &&cards);

private:
  Cards stack_;
};

} // namespace solitairecpp
