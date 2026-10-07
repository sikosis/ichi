#include "deck.hpp"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace ichi {

void Deck::buildStandard() {
    cards_.clear();
    const Color colors[] = {Color::Red, Color::Yellow, Color::Green, Color::Blue};
    for (Color color : colors) {
        cards_.push_back({color, Value::Zero});
        for (int n = 1; n <= 9; ++n) {
            Value v = static_cast<Value>(n);
            cards_.push_back({color, v});
            cards_.push_back({color, v});
        }
        for (Value v : {Value::Skip, Value::Reverse, Value::DrawTwo}) {
            cards_.push_back({color, v});
            cards_.push_back({color, v});
        }
    }
    for (int i = 0; i < 4; ++i) {
        cards_.push_back({Color::Wild, Value::Wild});
        cards_.push_back({Color::Wild, Value::WildDrawFour});
    }
}

void Deck::shuffle() {
    static std::mt19937 rng{std::random_device{}()};
    std::shuffle(cards_.begin(), cards_.end(), rng);
}

void Deck::add(const Card& c) {
    cards_.push_back(c);
}

void Deck::addAll(const std::vector<Card>& cards) {
    cards_.insert(cards_.end(), cards.begin(), cards.end());
}

Card Deck::draw() {
    if (cards_.empty()) {
        throw std::runtime_error("draw from empty deck");
    }
    Card c = cards_.back();
    cards_.pop_back();
    return c;
}

bool Deck::empty() const {
    return cards_.empty();
}

std::size_t Deck::size() const {
    return cards_.size();
}

std::vector<Card>& Deck::cards() {
    return cards_;
}

const std::vector<Card>& Deck::cards() const {
    return cards_;
}

} // namespace ichi
