#pragma once

#include <solitairecpp/board.hpp>
#include <solitairecpp/cards.hpp>
#include <solitairecpp/error.hpp>
#include <solitairecpp/move_manager.hpp>

namespace solitairecpp {

class ErrorExit : public ErrorBase {
public:
  std::string what() override { return "Exit was issued"; }

  Error error() override { return std::make_shared<ErrorExit>(); }
};

class Game {
public:
  Game();

private:
  std::expected<void, Error> chooseModeScreen();
  void mainLoop();

private:
  static const inline std::string splash_ = R"(
               ___ __        _
   _________  / (_) /_____ _(_)_______  _________  ____
  / ___/ __ \/ / / __/ __ `/ / ___/ _ \/ ___/ __ \/ __ \
 (__  ) /_/ / / / /_/ /_/ / / /  /  __/ /__/ /_/ / /_/ /
/____/\____/_/_/\__/\__,_/_/_/   \___/\___/ .___/ .___/
                                         /_/   /_/
)";
  static const inline ft::LinearGradient splash_gradient_ =
      ft::LinearGradient()
          .Angle(45)
          .Stop(ft::Color::RGB(122, 162, 247))
          .Stop(ft::Color::RGB(187, 154, 247))
          .Stop(ft::Color::RGB(125, 207, 255));
  Difficulty mode_{};
};

} // namespace solitairecpp
