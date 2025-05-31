#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/direction.hpp>
#include <ftxui/dom/elements.hpp>
#include <solitairecpp/solitairecpp.hpp>
#include <solitairecpp/utils.hpp>

namespace solitairecpp {

Game::Game() {
  auto success = chooseModeScreen();
  if (!success)
    return;
  mainLoop();
}

// function is large because of components. In ui component functions it will be
// natural.
std::expected<void, Error> Game::chooseModeScreen() {
  const std::vector<std::string> difficulties = {"Easy", "Hard"};
  int result{};
  auto onSelected = [&] {
    ft::ScreenInteractive::Active()->Exit();
    mode_ = static_cast<Difficulty>(result);
  };

  auto menu = ft::Menu(
      {.entries = &difficulties,
       .selected = &result,
       .entries_option = {.transform =
                              [](const ft::EntryState &state) {
                                ft::Element element = ft::text(state.label);
                                element |=
                                    ft::color(ft::Color::White) | ft::border;

                                if (state.focused)
                                  element |= ft::color(ft::Color::BlueLight);
                                return element;
                              }},
       .direction = ft::Direction::Right,
       .on_enter = onSelected,
       .focused_entry = &result});
  menu |= ft::CatchEvent([&](ft::Event event) {
    if (event.mouse().button == ft::Mouse::Left &&
        event.mouse().motion == ft::Mouse::Pressed) {
      onSelected();
      return true;
    }
    return false;
  });

  bool exit{};
  auto screen = ft::ScreenInteractive::Fullscreen();
  ft::Component modeScreen =
      ft::Container::Vertical({
          ft::Renderer(
              menu,
              [=, this] {
                return ft::vbox(
                    utils::text_split(splash_) | ft::color(splash_gradient_),
                    ft::separator() | ft::color(splash_gradient_),
                    ft::text("Please choose the difficulty") | ft::hcenter,
                    menu->Render() | ft::hcenter,
                    ft::separator() | ft::color(splash_gradient_));
              }),
      }) |
      ft::CatchEvent([&](ft::Event event) {
        if (event == ft::Event::CtrlC || event == ft::Event::Character('q')) {
          exit = true;
          ft::ScreenInteractive::Active()->Exit();
          return true;
        }
        return false;
      });

  screen.Loop(ft::Renderer(modeScreen,
                           [=] { return modeScreen->Render() | ft::center; }));
  if (exit)
    return std::unexpected(ErrorExit().error());

  return std::expected<void, Error>();
}

void Game::mainLoop() {
  auto screen = ft::ScreenInteractive::Fullscreen();
  Board board(mode_);
  auto boardComponent = board.component();
  screen.Loop(boardComponent);
}

} // namespace solitairecpp
