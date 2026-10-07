#pragma once

#include <string>

namespace ichi {

enum class Color {
    Red,
    Yellow,
    Green,
    Blue,
    Wild
};

enum class Value {
    Zero,
    One,
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine,
    Skip,
    Reverse,
    DrawTwo,
    Wild,
    WildDrawFour
};

struct Card {
    Color color;
    Value value;
};

bool isWild(const Card& c);
bool isNumber(const Card& c);

std::string valueLabel(Value v);
std::string colorName(Color c);
std::string cardLabel(const Card& c);
std::string colorAnsi(Color c);

} // namespace ichi
