#include <ftxui/component/screen_interactive.hpp>
#include <solitairecpp/solitairecpp.hpp>

namespace solitairecpp {

Game::Game() : moveManager_{boardElements_} {}

void Game::mainLoop() {
  auto screen = ft::ScreenInteractive::Fullscreen();
  auto board = boardElements_.component() |
               ft::CatchEvent(moveManager_.cardSelectedHandler());
  screen.Loop(board);
}

} // namespace solitairecpp
