#pragma once

#include <ftxui/component/component_base.hpp>
#include <solitairecpp/cards.hpp>
#include <utility>

namespace solitairecpp {

class MoveManager;

enum class Difficulty { Easy, Hard };

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

class Tableau {
public:
  static constexpr size_t startCardsSize = 28;
  typedef std::array<Card, startCardsSize> StartCards;

  struct CardPosition {
    size_t cardRowIndex{};
    size_t cardIndex{};
    bool operator==(const CardPosition &other) const;
  };

  // We always append at the end so a card position is unnecessary
  struct AppendCardPosition {
    size_t cardRowIndex{};

    bool operator==(const AppendCardPosition &other) const;
  };

public:
  Tableau(StartCards cards,
          MoveManager &moveManager); // Copying on purpose
  ft::Component component() const;
  std::expected<CardPosition, Error> search(const CardCode &code) const;
  std::expected<void, Error> appendTo(const AppendCardPosition &pos,
                                      const Cards &cards);
  std::expected<void, Error> deleteFrom(const CardPosition &pos);
  std::expected<Cards, Error> getCardsFrom(const CardPosition &pos);

  std::expected<void, Error> illegalAppendTo(const AppendCardPosition &pos,
                                             const Cards &cards);
  std::expected<bool, Error> isAppendToLegal(const AppendCardPosition &pos,
                                             const Cards &cards);

private:
  MoveManager &moveManager_;
  std::array<CardRow, 7> tableau_ =
      [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        return std::array<CardRow, 7>{
            {std::vector<CardRow>(sizeof...(Is), CardRow(Is, moveManager_))
                 .at(Is)...}};
      }(std::make_index_sequence<7>{});
};

class ReserveStack {
public:
  static constexpr size_t startCardsSize = 24;
  typedef std::array<Card, startCardsSize> StartCards;

  struct CardPosition {}; // it's empty because we always will take the top one
                          // no matter the difficulty

public:
  ReserveStack(Difficulty mode, StartCards cards); // Copying on purpose
  void reveal();
  ft::Component component();
  std::expected<CardPosition, Error> searchViewable(const CardCode &code);
  std::expected<Card, Error> getTopCard();
  std::expected<void, Error> deleteTopCard();
  std::expected<void, Error> setTopCard(const Card &card);

private:
  void moveToHiddenAndShuffle();
  void revealEasy();
  void revealHard();

private:
  static constexpr size_t hardDifficultyViewableAmount = 3;
  Difficulty mode_;
  Cards hiddenCards_;
  Cards viewedCards_;
  ft::Component viewableCardsComponent_;
};

class Foundations {
public:
  struct CardPosition {
    size_t foundationIndex;
  };

public:
  Foundations(MoveManager &moveManager, std::function<void()> onGameWon);
  std::expected<void, Error>
  set(const CardPosition &pos,
      const Card &card); // The previous card values just get deleted. In my
                         // vision that's what shoud happen.
  ft::Component component();

  // search is not directly needed but it eases
  // the job for classes that use it
  std::expected<CardPosition, Error> search(const CardCode &code);
  std::expected<bool, Error> isSetLegal(const CardPosition &pos,
                                        const Card &card);

private:
  ft::Component placeholder(size_t index);

private:
  std::array<Cards, 4> foundations_;
  ft::Component component_;
  MoveManager &moveManager_;
  std::function<void()> onGameWon_;
};

class ExitButton {
public:
  static ft::Component component();
};

typedef std::variant<Tableau::CardPosition, Tableau::AppendCardPosition,
                     ReserveStack::CardPosition, Foundations::CardPosition>
    CardPosition;

class Board {
public:
  Board(Difficulty mode, std::function<void()> onGameWon);
  // non-copyable
  Board(const Board &) = delete;
  Board &operator=(const Board &) = delete;

  std::expected<CardPosition, Error> search(const CardCode &code) const;
  ft::Component component() const;
  Cards buildDeck();

  ReserveStack &reserveStack() const;
  Tableau &tableau() const;
  Foundations &foundations() const;

private:
  std::unique_ptr<Tableau> tableau_ = nullptr;
  std::unique_ptr<ReserveStack> reserveStack_ = nullptr;
  std::unique_ptr<Foundations> foundations_ = nullptr;
  std::unique_ptr<MoveManager> moveManager_;
};

} // namespace solitairecpp
