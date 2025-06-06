#pragma once

#include "solitairecpp/leaderboard.hpp"
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
  Game() = default;
  void Start();

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
  static const inline std::string winSplash_ = R"(
__  __               _       __            __
\ \/ /___  __  __   | |     / /___  ____  / /
 \  / __ \/ / / /   | | /| / / __ \/ __ \/ /
 / / /_/ / /_/ /    | |/ |/ / /_/ / / / /_/
/_/\____/\__,_/     |__/|__/\____/_/ /_(_)

)";

  Difficulty mode_{};
  Leaderboard leaderboard_{};
};

} // namespace solitairecpp
