#pragma once

#include <memory>
#include <string>
#include <type_traits>

namespace solitairecpp {

class ErrorBase {
public:
  virtual std::string what() = 0;

  // Checks if the error is of a certain type
  template <typename Derived>
    requires std::is_base_of_v<ErrorBase, Derived>
  bool is(Derived) {
    Derived *d = dynamic_cast<Derived *>(this);
    return d == nullptr;
  }

  typedef std::shared_ptr<ErrorBase> Error; // Needs to be shared
  virtual Error error() = 0;                // Yes it will copy

  virtual ~ErrorBase() {};
};

typedef ErrorBase::Error Error;

} // namespace solitairecpp
