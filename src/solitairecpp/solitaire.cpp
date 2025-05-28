#include <ftxui/component/screen_interactive.hpp>
#include <solitairecpp/solitairecpp.hpp>

namespace solitairecpp {

void Game::mainLoop() {
  auto screen = ft::ScreenInteractive::Fullscreen();
  auto board = board_.component();
  screen.Loop(board);
}

} // namespace solitairecpp
