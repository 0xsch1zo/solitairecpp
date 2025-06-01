#include <algorithm>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <random>
#include <solitairecpp/board.hpp>
#include <solitairecpp/move_manager.hpp>
#include <solitairecpp/utils.hpp>

namespace solitairecpp {

ft::Component ExitButton::component() {
  return ft::Button({
      .label = "Exit Game",
      .on_click =
          [] {
            auto *screen = ft::ScreenInteractive::Active();
            if (screen == nullptr)
              throw std::runtime_error("Couldn't get active screen to exit");
            screen->Exit();
          },
  });
}

Board::Board(Difficulty mode, GameCallbacks callbacks)
    : moveManager_{std::make_unique<MoveManager>(*this)},
      gameCallbacks_{callbacks} {
  Cards deck = buildDeck();

  Tableau::StartCards tabelauCards =
      [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        return Tableau::StartCards{{deck.at(Is)...}};
      }(std::make_index_sequence<
          Tableau::startCardsSize>{}); // This is just initializing the array
                                       // with elements from the deck. Yes
                                       // there is problably no other simpler
                                       // way.
  tableau_ = std::make_unique<Tableau>(tabelauCards, *moveManager_);
  deck.erase(deck.begin(), deck.begin() + Tableau::startCardsSize);

  ReserveStack::StartCards reserveStackCards =
      [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        return ReserveStack::StartCards{{deck.at(Is)...}};
      }(std::make_index_sequence<ReserveStack::startCardsSize>{});
  reserveStack_ =
      std::make_unique<ReserveStack>(mode, *moveManager_, reserveStackCards);

  foundations_ =
      std::make_unique<Foundations>(*moveManager_, gameCallbacks_.onGameWon);
}

ft::Component Board::component() const {
  auto moveCounter = ft::Renderer([&] {
    return ft::text("Move count: " + std::to_string(moveManager_->moveCount()));
  });
  auto sidepanel = ft::Container::Vertical(
      {foundations_->component(), reserveStack_->component(), moveCounter,
       moveManager_->rollbackButton(),
       ft::Button("View leaderboard", gameCallbacks_.viewLeadearBoard,
                  ft::ButtonOption::Border()),
       ft::Button("Restart game", gameCallbacks_.restartGame,
                  ft::ButtonOption::Border()),
       ExitButton::component()});
  auto tableau = tableau_->component();

  auto board = ft::Container::Horizontal({sidepanel, tableau});
  return ft::Container::Horizontal({ft::Renderer(
             board,
             [=] {
               return ft::hbox(
                   ft::vbox(sidepanel->ChildAt(0)->Render(), ft::separator(),
                            sidepanel->ChildAt(1)->Render(), ft::separator(),
                            ft::filler(), sidepanel->ChildAt(2)->Render(),
                            sidepanel->ChildAt(3)->Render(),
                            sidepanel->ChildAt(4)->Render(),
                            sidepanel->ChildAt(5)->Render()),
                   ft::separator(), ft::filler(), tableau->Render());
             })}) |
         moveManager_->moveTransactionCanceledListener();
}

Cards Board::buildDeck() {
  const auto deckSize = 52;
  Cards deck;
  ArtGenerator generator;
  deck.reserve(deckSize);
  for (size_t i{}; i < static_cast<size_t>(CardValue::Count); i++) {
    for (size_t j{}; j < static_cast<size_t>(CardType::Count); j++) {
      auto value = static_cast<CardValue>(i);
      auto type = static_cast<CardType>(j);
      deck.emplace_back(*moveManager_, value, type,
                        generator.generate(value, type));
    }
  }

  std::random_device rd;
  std::mt19937 gen{rd()};

  std::shuffle(deck.begin(), deck.end(), gen);

  return deck;
}

std::expected<CardPosition, Error> Board::search(const CardCode &code) const {
  auto tableauPos = tableau_->search(code);
  if (tableauPos)
    return tableauPos.value();

  auto reserveStackPos = reserveStack_->searchViewable(code);
  if (reserveStackPos)
    return reserveStackPos.value();

  auto foundationsStackPos = foundations_->search(code);
  if (foundationsStackPos)
    return foundationsStackPos.value();

  return std::unexpected(ErrorCardPositionNotFound(code).error());
}

Tableau &Board::tableau() const { return *tableau_; }

ReserveStack &Board::reserveStack() const { return *reserveStack_; }

Foundations &Board::foundations() const { return *foundations_; }

size_t Board::moveCount() const { return moveManager_->moveCount(); }

} // namespace solitairecpp
