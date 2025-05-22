#pragma once

#include <string>

namespace solitairecpp {

class Error {
public:
  virtual std::string what() = 0;

  // Checks if the error is of a certain type
  template <typename Derived>
    requires std::is_base_of_v<Error, Derived>
  bool is(Derived) {
    Derived *d = dynamic_cast<Derived *>(this);
    return d == nullptr;
  }
};

} // namespace solitairecpp
