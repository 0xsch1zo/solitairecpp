#pragma once

#include <ftxui/component/component.hpp>
#include <vector>

namespace ft = ftxui;

namespace solitairecpp {

class Leaderboard {
public:
  struct LeaderboardEntry {
    size_t gameNumber{};
    size_t moveCount{};
  };

  void registerScore(size_t moveCount);

  ft::Component component();

private:
  static const inline std::string leaderboardHeader_ = R"(
    __                   __          __                         __
   / /   ___  ____ _____/ /__  _____/ /_  ____  ____ __________/ /
  / /   / _ \/ __ `/ __  / _ \/ ___/ __ \/ __ \/ __ `/ ___/ __  /
 / /___/  __/ /_/ / /_/ /  __/ /  / /_/ / /_/ / /_/ / /  / /_/ /
/_____/\___/\__,_/\__,_/\___/_/  /_.___/\____/\__,_/_/   \__,_/

)";
  std::vector<LeaderboardEntry> leaderboard_;
};

} // namespace solitairecpp
