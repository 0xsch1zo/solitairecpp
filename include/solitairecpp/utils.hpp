#pragma once

#include <ftxui/dom/elements.hpp>
#include <string>

namespace ft = ftxui;

namespace solitairecpp {
class utils {
public:
  static ft::Elements text_split(const std::string &text);
};

} // namespace solitairecpp
