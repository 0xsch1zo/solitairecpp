#include <ftxui/component/event.hpp>
#include <solitairecpp/move_manager.hpp>
#include <stdexcept>

namespace solitairecpp {

MoveManager::MoveManager(const Board &elements) : boardElements_{elements} {}

bool MoveManager::isBeingMoved(const CardCode &code) const {
  const auto &from = moveFrom_.load();
  if (!from.has_value())
    return false;
  auto position = boardElements_.search(code);
  if (!position)
    throw std::runtime_error(position.error()->what());

  if (from.value().index() != position.value().index())
    return false;

  // This is the single greatest thing in the universe
  return std::visit(
      [](const auto &fromPosition, const auto &queriedPosition) {
        return fromPosition == queriedPosition;
      },
      from.value(), position.value());
}

std::expected<void, Error> MoveManager::Move(const CardPosition &from,
                                             const CardPosition &to) {
  if (std::holds_alternative<Tableau::CardPosition>(from)) {
    if (std::holds_alternative<Tableau::CardPosition>(to)) {
      auto cards = boardElements_.tableau().getCardsFrom(
          std::get<Tableau::CardPosition>(from));
      if (!cards)
        throw std::runtime_error(cards.error()->what());

      auto deleteSuccess = boardElements_.tableau().deleteFrom(
          std::get<Tableau::CardPosition>(from));
      if (!deleteSuccess)
        throw std::runtime_error(deleteSuccess.error()->what());

      auto appendSuccess = boardElements_.tableau().appendTo(
          {.cardRowIndex = std::get<Tableau::CardPosition>(to).cardRowIndex},
          cards.value());
      if (!appendSuccess)
        throw std::runtime_error(appendSuccess.error()->what());
    }
  }

  return std::expected<void, Error>();
}

std::function<bool(ft::Event)> MoveManager::cardSelectedHandler() {
  return [this](ft::Event event) -> bool {
    auto card = CardSerializer::Decode(event.input());
    if (!card)
      return false;

    // Find the thing
    if (moveFrom_.load() == std::nullopt) {
      auto position = boardElements_.search(card.value());
      if (!position)
        throw std::runtime_error(position.error()->what());

      moveFrom_ = position.value();
    } else {
      auto position = boardElements_.search(card.value());
      if (!position)
        throw std::runtime_error(position.error()->what());

      auto success = Move(moveFrom_.load().value(), position.value()); // handle
      if (!success)
        throw std::runtime_error(success.error()->what());

      moveFrom_ = std::nullopt;
    }
    return true;
  };
}

} // namespace solitairecpp
