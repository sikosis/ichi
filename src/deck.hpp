#pragma once

#include <cstddef>
#include <vector>

#include "card.hpp"

namespace ichi {

class Deck {
public:
    void buildStandard();
    void shuffle();
    void add(const Card& c);
    void addAll(const std::vector<Card>& cards);
    Card draw();
    bool empty() const;
    std::size_t size() const;
    std::vector<Card>& cards();
    const std::vector<Card>& cards() const;

private:
    std::vector<Card> cards_;
};

} // namespace ichi
