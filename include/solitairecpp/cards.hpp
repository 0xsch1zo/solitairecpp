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

enum class CardColor { Red, Black };

// Due to each card in the deck being unique we can idenitfy them by thier value
// combined with thier type/color.
struct CardCode {
  CardValue value;
  CardType type;
  bool operator==(const CardCode &rhs) const;
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

class ErrorCardPositionNotFound : public ErrorBase {
public:
  ErrorCardPositionNotFound(const CardCode &code) : code_{code} {}

  std::string what() override {
    return std::format("Card with the supplied code was not found: {} {}",
                       static_cast<size_t>(code_.value),
                       static_cast<size_t>(code_.type));
  }

  Error error() override {
    return std::make_shared<ErrorCardPositionNotFound>(code_);
  }

private:
  CardCode code_;
};

class Card {
public:
  Card(MoveManager &moveManager, CardValue value, CardType type,
       std::string art, bool hidden = true);
  Card &operator=(const Card &other);

  void show();
  CardCode code() const;
  ft::Component component() const;
  CardColor color() const;

  // Tempororary remove at release
  std::string getArt() const { return art_; }

  static inline const auto cardWidth = ft::size(ft::WIDTH, ft::EQUAL, 15);
  static inline const auto cardHeight = ft::size(ft::HEIGHT, ft::EQUAL, 4);

private:
  std::shared_ptr<bool> hidden_;
  CardValue value_;
  CardType type_;
  CardColor color_;
  std::string art_ = "art not initalized";
  ft::Component component_;
  MoveManager &moveManager_;
};

typedef std::vector<Card> Cards;

// wrapper around std::vector
class CardRow {
public:
  struct CardPosition {
    size_t cardIndex;
  };

public:
  CardRow(size_t index, MoveManager &moveManager);
  ft::Component component() const;
  std::expected<void, Error> append(const Cards &cards);
  std::expected<void, Error> deleteFrom(const CardPosition &pos);
  std::expected<Cards, Error> getCardsFrom(const CardPosition &pos);
  std::expected<CardPosition, Error> search(const CardCode &code) const;

  bool isAppendLegal(const Cards &cards);
  void illegalAppend(const Cards &cards);

private:
  ft::Component cardsComponent_;
  Cards cards_;
  size_t index_;
  MoveManager &moveManager_;
};

} // namespace solitairecpp
