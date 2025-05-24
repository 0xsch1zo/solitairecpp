#include <ftxui/component/event.hpp>
#include <solitairecpp/cards.hpp>
#include <solitairecpp/logic.hpp>

namespace solitairecpp {

bool Logic::cardSelectedHandler(ft::Event event) {
  auto card = CardSerializer::Decode(event.input());
  if (!card)
    return false;

  return true;
}

} // namespace solitairecpp
