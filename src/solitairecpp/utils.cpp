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

}; // namespace solitairecpp
