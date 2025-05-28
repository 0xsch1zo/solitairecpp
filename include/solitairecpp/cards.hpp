#pragma once

#include <expected>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>
#include <solitairecpp/error.hpp>
#include <string>
#include <vector>

namespace ft = ftxui;

namespace solitairecpp {

class MoveManager;

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
// combined with thier type/color.
struct CardCode {
  CardValue value;
  CardType type;
  bool operator==(const CardCode &rhs) const;
};

class CardSerializer {
public:
  static std::string Encode(const CardCode &cardCode);

  static std::expected<CardCode, Error>
  Decode(const std::string &serializedCard);

private:
  static constexpr std::string magic_ = "CardCode: ";
  static constexpr std::string delimeter_ = "x";
};

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

class ErrorCardPositionNotFound : public ErrorBase {
public:
  ErrorCardPositionNotFound(const CardCode &code) : code_{code} {}

  std::string what() override {
    return std::format("Card with the supplied code was not found: {}",
                       CardSerializer::Encode(code_));
  }

  Error error() override {
    return std::make_shared<ErrorCardPositionNotFound>(code_);
  }

private:
  CardCode code_;
};

class Card {
public:
  Card(const MoveManager &moveManager, CardValue value, CardType type,
       std::string art, bool hidden = true);
  Card &operator=(const Card &other);

  CardCode code() const;
  ft::Component component() const;

  // Tempororary remove at release
  std::string getArt() const { return art_; }

private:
  static inline const auto cardWidth = ft::size(ft::WIDTH, ft::EQUAL, 20);
  static inline const auto cardHeight = ft::size(ft::HEIGHT, ft::EQUAL, 7);
  bool hidden_;
  CardValue value_;
  CardType type_;
  std::string art_ = "art not initalized";
  ft::Component component_;
  const MoveManager &moveManager_;
};

typedef std::vector<Card> Cards;

// wrapper around std::vector
class CardRow {
public:
  struct CardPosition {
    size_t cardIndex;
  };

public:
  CardRow();
  ft::Component component() const;
  std::expected<void, Error> append(const Cards &cards);
  std::expected<void, Error> deleteFrom(const CardPosition &pos);
  std::expected<Cards, Error> getCardsFrom(const CardPosition &pos);
  std::expected<CardPosition, Error> search(const CardCode &code) const;

private:
  ft::Component component_;
  Cards cards_;
};

} // namespace solitairecpp
