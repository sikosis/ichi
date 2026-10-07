#include "player.hpp"

#include <utility>

namespace ichi {

Player::Player(std::string name, bool ai)
    : name_(std::move(name)), ai_(ai) {}

const std::string& Player::name() const {
    return name_;
}

bool Player::isAI() const {
    return ai_;
}

std::vector<Card>& Player::hand() {
    return hand_;
}

const std::vector<Card>& Player::hand() const {
    return hand_;
}

void Player::addCard(const Card& c) {
    hand_.push_back(c);
}

bool Player::playCard(std::size_t index, Card& out) {
    if (index >= hand_.size()) {
        return false;
    }
    out = hand_[index];
    hand_.erase(hand_.begin() + static_cast<std::ptrdiff_t>(index));
    return true;
}

int Player::count() const {
    return static_cast<int>(hand_.size());
}

bool Player::hasCard() const {
    return !hand_.empty();
}

bool Player::saidIchi() const {
    return saidIchi_;
}

void Player::setSaidIchi(bool value) {
    saidIchi_ = value;
}

} // namespace ichi
