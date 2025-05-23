#include <algorithm>
#include <array>
#include <memory>
#include <print>
#include <random>
#include <solitairecpp/cards.hpp>
#include <solitairecpp/solitairecpp.hpp>

namespace solitairecpp {

Game::Game() {
  Cards deck = buildDeck();
  for (const auto &card : deck)
    std::println("{}", card.getArt());
  std::println("----------------------");

  StartTableauCards tabelauCards;
  std::copy(deck.begin(), deck.begin() + tabelauCards.size(),
            tabelauCards.begin());
  tableau_ = std::make_unique<Tableau>(std::move(tabelauCards));

  StartReserveStackCards reserveStackCards;
  std::copy(deck.begin() + tabelauCards.size(), deck.end(),
            reserveStackCards.begin());
  reserveStack_ = std::make_unique<ReserveStack>(std::move(reserveStackCards));
}

Cards Game::buildDeck() {
  const auto deckSize = 52;
  Cards deck;
  deck.reserve(deckSize);
  for (size_t i{}; i < static_cast<size_t>(CardValue::Count); i++) {
    for (size_t j{}; j < static_cast<size_t>(CardType::Count); j++) {
      auto value = static_cast<CardValue>(i);
      auto type = static_cast<CardType>(j);
      deck.emplace_back(value, type,
                        "test " + std::to_string(i) + " " + std::to_string(j));
    }
  }

  std::random_device rd;
  std::mt19937 gen{rd()};

  std::shuffle(deck.begin(), deck.end(), gen);

  return deck;
}

void Game::mainLoop() {}

} // namespace solitairecpp
