#include <atomic>
#include <solitairecpp/board.hpp>
#include <solitairecpp/error.hpp>

namespace solitairecpp {

class MoveManager {
public:
  MoveManager(const BoardElements &elements);
  std::function<bool(ft::Event)> cardSelectedHandler();
  bool isBeingMoved(const CardCode &code) const;

private:
  std::expected<void, Error> Move(const CardPosition &from,
                                  const CardPosition &to);

private:
  const BoardElements &boardElements_;
  std::atomic<std::optional<CardPosition>>
      moveFrom_; // only when move sequence is initiated
};

} // namespace solitairecpp
