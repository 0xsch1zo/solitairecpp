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
  bool is() {
    return dynamic_cast<Derived *>(this) != nullptr;
  }

  typedef std::shared_ptr<ErrorBase>
      Error; // Needs to be shared because when std::expected throws it needs to
             // copy
  virtual Error error() = 0; // Yes it will copy

  virtual ~ErrorBase() {};
};

typedef ErrorBase::Error Error;

} // namespace solitairecpp
