#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <print>
#include <solitairecpp/cards.hpp>
#include <solitairecpp/error.hpp>
#include <solitairecpp/move_manager.hpp>
#include <unordered_map>

namespace solitairecpp {

bool CardCode::operator==(const CardCode &rhs) const {
  return value == rhs.value && type == rhs.type;
}

Card::Card(MoveManager &moveManager, CardValue value, CardType type,
           std::string art, bool hidden)
    : moveManager_{moveManager}, value_{value}, type_{type}, art_{art},
      hidden_{std::make_shared<bool>(hidden)} {
  if (type_ == CardType::Hearts || type_ == CardType::Diamonds)
    color_ = CardColor::Red;
  else
    color_ = CardColor::Black;
  component_ = ft::Button(
      {.on_click =
           [=, *this] {
             if (*hidden_)
               return;

             std::thread([*this] {
               moveManager_.cardSelected(code());
             }).detach();
           },
       .transform =
           [=, *this](const ft::EntryState state) {
             auto element =
                 ft::text(*hidden_ ? backsideArt_ : art_) | ft::center;
             element |= cardWidth | cardHeight;
             element |= ft::border;

             if (!*hidden_) {
               switch (color_) {
               case CardColor::Red:
                 element |= ft::color(ft::Color::Red);
                 break;
               case CardColor::Black:
                 element |= ft::color(ft::Color::GrayDark);
                 break;
               }
             }

             // shoud not focus if the transaction is open or the
             // card is hidden, unless we are targetable;
             if (moveManager_.moveTransactionOpen() || *hidden_)
               return element;

             if (state.active) {
               element |= ft::bold;
             }
             if (state.focused) {
               element |= ft::inverted;
             }
             return element;
           }});
}

Card &Card::operator=(const Card &other) {
  if (this == &other)
    return *this;

  value_ = other.value_;
  type_ = other.type_;
  art_ = other.art_;
  hidden_ = other.hidden_;
  component_ = other.component_;
  color_ = other.color_;
  return *this;
}

void Card::showWithoutStatusChange() { *hidden_ = false; }

void Card::show() {
  if (*hidden_) { // To presereve hiddenStatusChanged
    hiddenStatusChanged = true;
    *hidden_ = false;
  }
}

void Card::hideRollback() {
  if (hiddenStatusChanged) // Only when a card's status was changed by the user
                           // we can hide it again
    *hidden_ = true;
}

ft::Component Card::component() const { return std::move(component_); }

CardCode Card::code() const { return {.value = value_, .type = type_}; }

CardColor Card::color() const { return color_; }

CardRow::CardRow(size_t index, MoveManager &moveManager)
    : cardsComponent_{ft::Container::Vertical({})}, moveManager_{moveManager},
      index_{index} {}

bool CardRow::isAppendLegal(const Cards &tobeappended) {
  if (tobeappended.empty())
    return false; // empty sequence should not get appended

  // only sequence starting with a king can get appened
  if (cards_.empty() && tobeappended.front().code().value != CardValue::King)
    return false;

  // the first one can't be the same color
  if (!cards_.empty() && cards_.back().color() == tobeappended.front().color())
    return false;

  // value needs to be lower than previous if's ace there will never be a card
  // with -1 index, so everything works out
  if (!cards_.empty() &&
      static_cast<int>(cards_.back().code().value) - 1 !=
          static_cast<int>(tobeappended.front().code().value))
    return false;

  if (tobeappended.size() == 1)
    return true; // nothing left to validate

  auto previousColor = tobeappended.front().color();
  auto previousValue = tobeappended.front().code().value;
  for (size_t i{1}; i < tobeappended.size(); i++) {
    const auto &card = tobeappended.at(i);
    if (static_cast<int>(card.code().value) !=
        static_cast<int>(previousValue) - 1)
      return false;

    if (card.color() == previousColor)
      return false;

    previousValue = card.code().value;
    previousColor = card.color();
  }

  return true; // finally
}

void CardRow::appendCore(const Cards &cards) {
  if (cards_.size() == 0 && cardsComponent_->ChildCount() == 1)
    cardsComponent_->DetachAllChildren(); // remove dummy

  cards_.reserve(cards.size());
  for (const auto &card : cards) {
    cards_.emplace_back(card);
    cardsComponent_->Add(card.component());
  }
}

std::expected<void, Error> CardRow::append(const Cards &cards) {
  if (!isAppendLegal(cards))
    return std::unexpected(ErrorIllegalMove().error());

  appendCore(cards);
  return std::expected<void, Error>();
}

void CardRow::appendRollback(const Cards &cards) {
  if (!cards_.empty())
    cards_.back().hideRollback(); // Hide the previous one

  appendCore(cards);
}

std::expected<void, Error> CardRow::deleteFrom(const CardPosition &pos) {
  if (pos.cardIndex >= cards_.size())
    return std::unexpected(ErrorInvalidCardIndex().error());

  for (size_t i{pos.cardIndex}; i < cards_.size(); i++)
    cardsComponent_->ChildAt(pos.cardIndex)->Detach();

  cards_.erase(cards_.begin() + pos.cardIndex, cards_.end());

  if (!cards_.empty())
    cards_.back().show(); // Reveal the last card

  if (cards_.empty()) {
    // Add dummy so that "Empty container" doesn't get displayed
    cardsComponent_->Add(
        ft::Renderer([] { return ft::emptyElement() | Card::cardWidth; }));
  }

  return std::expected<void, Error>();
}

ft::Component CardRow::component() const {
  auto moveTargetBar = ft::Button(
      {.on_click =
           [this] {
             if (!moveManager_.moveTransactionOpen())
               return;
             moveManager_.setMoveTarget(Tableau::CardPosition{
                 .cardRowIndex = index_,
                 .cardIndex = cards_.size()}); // will point to the card that's
                                               // about to beadded
           },
       .transform =
           [this](const ft::EntryState &state) {
             auto element = ft::separator();

             // should not focus if transaction is not open
             if (!moveManager_.moveTransactionOpen())
               return element;

             if (state.active)
               element |= ft::bold;
             if (state.focused)
               element |= ft::inverted;

             if (moveManager_.isTargetError(Tableau::CardPosition{
                     .cardRowIndex = index_, .cardIndex = cards_.size()}))
               element |= ft::color(ft::Color::Red);

             return element | ft::color(ft::Color::Green);
           }});
  return std::move(ft::Container::Vertical({cardsComponent_, moveTargetBar}));
}

std::expected<CardRow::CardPosition, Error>
CardRow::search(const CardCode &code) const {
  for (const auto [i, card] :
       std::views::zip(std::views::iota(0ULL), cards_)) { // C++23 baby
    if (code == card.code())
      return CardPosition{.cardIndex = i};
  }

  return std::unexpected(ErrorCardPositionNotFound(code).error());
}

std::expected<Cards, Error> CardRow::getCardsFrom(const CardPosition &pos) {
  if (pos.cardIndex >= cards_.size())
    return std::unexpected(ErrorInvalidCardIndex().error());

  Cards res(cards_.begin() + pos.cardIndex, cards_.end());
  return res;
}

std::string ArtGenerator::generate(CardValue value, CardType type) {
  return valueStr.at(value) + " " + typeStr.at(type);
}

} // namespace solitairecpp
