#pragma once

#include <string>

namespace solitairecpp {

enum class ErrorType { InvalidCardRange, InvalidCardIndex };

class Error {
public:
  virtual std::string what() = 0;
  virtual ErrorType type() = 0;

  // Checks if the error is of a certain type
  template <typename Derived>
    requires std::is_base_of_v<Error, Derived>
  bool is(Derived) {
    Derived *d = dynamic_cast<Derived *>(this);
    return d == nullptr;
  }
};

} // namespace solitairecpp
