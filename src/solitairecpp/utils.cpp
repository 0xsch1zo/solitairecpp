#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <solitairecpp/utils.hpp>

namespace solitairecpp {

ft::Elements utils::textSplit(const std::string &text) {
  ft::Elements output;
  std::stringstream ss(text);
  std::string line;
  output.reserve(text.size());
  while (std::getline(ss, line, '\n'))
    output.emplace_back(ft::paragraph(line));
  return output;
}

ft::ComponentDecorator utils::exitListener() {
  return ft::CatchEvent([](ft::Event event) {
    if (event == ft::Event::Character('q') || event == ft::Event::CtrlC) {
      ft::ScreenInteractive::Active()->Exit();
      return true;
    }
    return false;
  });
}

}; // namespace solitairecpp
