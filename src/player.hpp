#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "card.hpp"

namespace ichi {

class Player {
public:
    Player(std::string name, bool ai);

    const std::string& name() const;
    bool isAI() const;

    std::vector<Card>& hand();
    const std::vector<Card>& hand() const;

    void addCard(const Card& c);
    bool playCard(std::size_t index, Card& out);
    int count() const;
    bool hasCard() const;

    bool saidIchi() const;
    void setSaidIchi(bool value);

private:
    std::string name_;
    bool ai_;
    std::vector<Card> hand_;
    bool saidIchi_ = false;
};

} // namespace ichi
