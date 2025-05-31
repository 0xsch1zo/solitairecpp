#include <atomic>
#include <ftxui/component/component.hpp>
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
  void setMoveTarget(const CardCode &code);
  void setMoveTarget(const CardPosition &pos);

  bool isMoveTarget(const CardPosition &pos) const;
  bool isTargetError(const CardPosition &pos) const;
  bool moveTransactionOpen() const;
  bool isRollbackBlocked() const;
  size_t moveCount() const;

  void rollback();
  ft::ComponentDecorator moveTransactionCanceledListener();

private:
  struct moveTransaction {
    CardPosition from;
    CardPosition to;
  };

private:
  std::expected<void, Error> Move();
  // Each overload describes a posssible origin and destination
  std::expected<void, Error> moveHelper(const Tableau::CardPosition &from,
                                        const Tableau::CardPosition &to);
  std::expected<void, Error> moveHelper(const Tableau::CardPosition &from,
                                        const Foundations::CardPosition &to);
  std::expected<void, Error> moveHelper(const ReserveStack::CardPosition &from,
                                        const Tableau::CardPosition &to);
  std::expected<void, Error> moveHelper(const ReserveStack::CardPosition &from,
                                        const Foundations::CardPosition &to);

  // Yes the equivalents for rollbacks are needed and aren't just repetition
  void rollbackHelper(const Tableau::CardPosition &from,
                      const Tableau::CardPosition &to);
  void rollbackHelper(const Foundations::CardPosition &from,
                      const Tableau::CardPosition &to);
  void rollbackHelper(const Tableau::CardPosition &from,
                      const ReserveStack::CardPosition &to);
  void rollbackHelper(const Foundations::CardPosition &from,
                      const ReserveStack::CardPosition &to);
  void endTransaction();

private:
  static constexpr size_t maxHistorySize_ = 3;
  std::vector<moveTransaction> history_;
  const Board &board_;
  std::atomic<std::optional<CardPosition>>
      moveFrom_; // only when move sequence is initiated
  std::atomic<std::optional<CardPosition>> moveTo_;
  std::atomic<std::optional<CardPosition>> erroneusTarget_;
  std::atomic<size_t> moveCount_{};
};

} // namespace solitairecpp
