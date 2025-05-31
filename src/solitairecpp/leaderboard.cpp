#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <solitairecpp/leaderboard.hpp>
#include <solitairecpp/utils.hpp>
#include <string>

namespace solitairecpp {

void Leaderboard::registerScore(size_t moveCount) {
  leaderboard_.emplace_back(leaderboard_.size(), moveCount);
  std::sort(leaderboard_.begin(), leaderboard_.end(),
            [](auto a, auto b) { return a.moveCount < b.moveCount; });
}

ft::Component Leaderboard::component() {
  std::vector<std::vector<std::string>> tableInput;
  tableInput.reserve(leaderboard_.size());
  tableInput.push_back({"Game ID", "Move count"}); // header
  for (const auto &entry : leaderboard_)
    tableInput.push_back(
        {std::to_string(entry.gameNumber), std::to_string(entry.moveCount)});

  auto leaderboardTable = ft::Table(tableInput);
  leaderboardTable.SelectAll().Border(ft::ROUNDED);
  leaderboardTable.SelectRow(0).Decorate(ft::bold);
  leaderboardTable.SelectColumns(0, 1).SeparatorHorizontal(ft::HEAVY);
  leaderboardTable.SelectColumns(0, 1).SeparatorVertical(ft::HEAVY);
  auto leaderboardElement = leaderboardTable.Render();

  auto upperLeaderBoardscreen = ft::Renderer([=, *this] {
    auto header =
        ft::vbox(utils::textSplit(leaderboardHeader_), ft::separator()) |
        ft::color(utils::headerGradient);
    if (leaderboard_.empty())
      return ft::vbox(
          header,
          ft::text("The leaderboard is empty play a game to fill it up!") |
              ft::hcenter);

    return ft::vbox(header, leaderboardElement | ft::hcenter);
  });

  auto bottomLeaderboardScreen =
      ft::Button({.label = "Exit",
                  .on_click = [] { ft::ScreenInteractive::Active()->Exit(); },
                  .transform =
                      [](const ft::EntryState &state) {
                        auto element =
                            ft::text(state.label) | ft::center | ft::border;
                        if (state.active)
                          element |= ft::bold;
                        if (state.focused)
                          element |= ft::inverted;
                        return element;
                      }});
  return ft::Container::Vertical(
             {upperLeaderBoardscreen, bottomLeaderboardScreen}) |
         utils::exitListener();
}

} // namespace solitairecpp
