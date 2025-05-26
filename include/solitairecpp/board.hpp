#pragma once

#include <solitairecpp/cards.hpp>
#include <utility>

namespace solitairecpp {

enum class BoardSection {
  Tableau,
  ReserveStack,
  Foundations,
};

// useful for debug and errors
static const char *BoardSectionStr(BoardSection section) {
  switch (section) {
  case BoardSection::Tableau:
    return "Tableau";
  case BoardSection::ReserveStack:
    return "ReserveStack";
  case BoardSection::Foundations:
    return "Foundtations";
  default:
    std::unreachable();
  }
  std::unreachable();
};

class ErrorWrongSection : public ErrorBase {
public:
  ErrorWrongSection(BoardSection section) : section_{section} {}
  std::string what() override {
    return std::format("The card searched for is in section: {}",
                       BoardSectionStr(section_));
  }

  Error error() override {
    return std::make_shared<ErrorWrongSection>(section_);
  }

private:
  BoardSection section_;
};

typedef std::array<Card, 28>
    StartTableauCards; // The main table has 28 cards at the start

class Tableau {
public:
  struct CardPosition {
    size_t cardRowIndex{};
    size_t cardIndex{};
  };

  // We always append at the end so a card position is unnecessary
  struct AppendCardPosition {
    size_t cardRowIndex{};
  };

public:
  Tableau(StartTableauCards cards); // Copying on purpose
  ft::Component component() const;
  std::expected<CardPosition, Error> search(const CardCode &code) const;
  std::expected<void, Error> appendTo(const AppendCardPosition &pos,
                                      const Cards &cards);
  std::expected<void, Error> deleteFrom(const CardPosition &pos);

private:
  std::array<CardRow, 7> tableau_;
};

typedef std::array<Card, 24>
    StartReserveStackCards; // The reserve stack has 24 cards at the start

class ReserveStack {
public:
  ReserveStack(StartReserveStackCards cards); // Copying on purpose

private:
  Cards stack_;
};

typedef std::variant<Tableau::CardPosition> CardPosition;

struct BoardElements {
public:
  BoardElements();
  // non-copyable
  BoardElements(const BoardElements &) = delete;
  BoardElements &operator=(const BoardElements &) = delete;

  std::expected<CardPosition, Error> search(const CardCode &code) const;
  ft::Component component() const;

  std::unique_ptr<ReserveStack> reserveStack_ = nullptr;
  std::unique_ptr<Tableau> tableau_ = nullptr;

public:
  Cards buildDeck();
};

} // namespace solitairecpp
