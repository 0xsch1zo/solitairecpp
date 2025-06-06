#pragma once

#include <ftxui/component/component_base.hpp>
#include <functional>
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
  std::expected<void, Error> appendToRollback(const AppendCardPosition &pos,
                                              const Cards &cards);
  std::expected<void, Error> deleteFrom(const CardPosition &pos);
  std::expected<Cards, Error> getCardsFrom(const CardPosition &pos);

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
  ReserveStack(Difficulty mode, MoveManager &moveManager,
               StartCards cards); // Copying on purpose
  void reveal();
  ft::Component component();
  std::expected<CardPosition, Error> searchViewable(const CardCode &code);
  std::expected<Card, Error> getTopCard();
  std::expected<void, Error> deleteTopCard();
  std::expected<void, Error> setTopCard(const Card &card);
  std::expected<void, Error> rollbackCard();

private:
  class ReserveStackCard : public Card {
  public:
    ReserveStackCard(const Card &card, const ReserveStack &reserveStack);
    ft::Component component() const override;

  private:
    const ReserveStack &reserveStack_;
  };

private:
  void moveToHiddenAndShuffle();
  ft::Component placeholder();
  void revealEasy();
  void revealHard();

private:
  static constexpr size_t hardDifficultyViewableAmount = 3;
  Difficulty mode_;
  Cards hiddenCards_;
  Cards viewedCards_;
  ft::Component viewableCardsComponent_;
  MoveManager &moveManager_;
};

class Foundations {
public:
  struct CardPosition {
    size_t foundationIndex;
  };

public:
  Foundations(MoveManager &moveManager, std::function<void()> onGameWon);
  std::expected<Card, Error> acquireCard(const CardPosition &pos);
  std::expected<void, Error> deleteCard(const CardPosition &pos);
  std::expected<void, Error> setCard(const CardPosition &pos, const Card &card);
  ft::Component component();

  // search is not directly needed but it eases
  // the job for classes that use it
  std::expected<CardPosition, Error> search(const CardCode &code);
  std::expected<bool, Error> isSetLegal(const CardPosition &pos,
                                        const Card &card);

private:
  class FoundationCard : public Card {
  public:
    FoundationCard(const Card &card);
    ft::Component component() const override;
  };

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

// Abstraction(kind of) over card positions for move manager
typedef std::variant<Tableau::CardPosition, ReserveStack::CardPosition,
                     Foundations::CardPosition>
    CardPosition;

class Board {
public:
  struct GameCallbacks {
    std::function<void()> onGameWon;
    std::function<void()> restartGame;
    std::function<void()> viewLeadearBoard;
  };

public:
  Board(Difficulty mode, GameCallbacks callbacks);
  // non-copyable
  Board(const Board &) = delete;
  Board &operator=(const Board &) = delete;

  std::expected<CardPosition, Error> search(const CardCode &code) const;
  ft::Component component() const;
  Cards buildDeck();

  ReserveStack &reserveStack() const;
  Tableau &tableau() const;
  Foundations &foundations() const;

  size_t moveCount() const;

private:
  std::unique_ptr<Tableau> tableau_ = nullptr;
  std::unique_ptr<ReserveStack> reserveStack_ = nullptr;
  std::unique_ptr<Foundations> foundations_ = nullptr;
  std::unique_ptr<MoveManager> moveManager_;
  GameCallbacks gameCallbacks_;
};

} // namespace solitairecpp
