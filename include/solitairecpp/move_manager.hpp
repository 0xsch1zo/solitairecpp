#include <atomic>
#include <solitairecpp/board.hpp>
#include <solitairecpp/cards.hpp>
#include <solitairecpp/error.hpp>

namespace solitairecpp {

class ErrorIllegalMove : public ErrorBase {
public:
  std::string what() override { return "Tried to perform an illegal move"; }

  Error error() override { return std::make_shared<ErrorIllegalMove>(); }
};

class MoveManager {
public:
  MoveManager(const Board &elements);

  void cardSelected(const CardCode &code);
  void setMoveTarget(const CardPosition &pos);

  bool isBeingMoved(const CardCode &code) const;
  bool isMoveTarget(const CardPosition &pos) const;
  bool isTargetError(const CardPosition &pos) const;
  bool moveTransactionOpen() const;

private:
  std::expected<void, Error> Move();
  std::expected<void, Error> moveHelper(const Tableau::CardPosition &from,
                                        const Tableau::AppendCardPosition &to);
  std::expected<void, Error> moveHelper(const Tableau::CardPosition &from,
                                        const Foundations::CardPosition &to);
  std::expected<void, Error> moveHelper(const ReserveStack::CardPosition &from,
                                        const Tableau::AppendCardPosition &to);

private:
  const Board &board_;
  std::atomic<std::optional<CardPosition>>
      moveFrom_; // only when move sequence is initiated
  std::atomic<std::optional<CardPosition>> moveTo_;
  std::atomic<std::optional<CardPosition>> erroneusTarget_;
};

} // namespace solitairecpp
