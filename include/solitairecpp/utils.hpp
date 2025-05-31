#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>

namespace ft = ftxui;

namespace solitairecpp {
class utils {
public:
  static ft::Elements textSplit(const std::string &text);

  static ft::ComponentDecorator exitListener();

  static const inline ft::LinearGradient headerGradient =
      ft::LinearGradient()
          .Angle(45)
          .Stop(ft::Color::RGB(122, 162, 247))
          .Stop(ft::Color::RGB(187, 154, 247))
          .Stop(ft::Color::RGB(125, 207, 255));
};

} // namespace solitairecpp
