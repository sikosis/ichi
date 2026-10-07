#include "card.hpp"

namespace ichi {

bool isWild(const Card& c) {
    return c.value == Value::Wild || c.value == Value::WildDrawFour;
}

bool isNumber(const Card& c) {
    return c.value >= Value::Zero && c.value <= Value::Nine;
}

std::string valueLabel(Value v) {
    switch (v) {
    case Value::Zero:  return "0";
    case Value::One:   return "1";
    case Value::Two:   return "2";
    case Value::Three: return "3";
    case Value::Four:  return "4";
    case Value::Five:  return "5";
    case Value::Six:   return "6";
    case Value::Seven: return "7";
    case Value::Eight: return "8";
    case Value::Nine:  return "9";
    case Value::Skip:  return "S";
    case Value::Reverse: return "R";
    case Value::DrawTwo: return "+2";
    case Value::Wild:  return "W";
    case Value::WildDrawFour: return "+4";
    }
    return "?";
}

std::string colorName(Color c) {
    switch (c) {
    case Color::Red:    return "Red";
    case Color::Yellow: return "Yellow";
    case Color::Green:  return "Green";
    case Color::Blue:   return "Blue";
    case Color::Wild:   return "Wild";
    }
    return "?";
}

std::string cardLabel(const Card& c) {
    if (isWild(c)) {
        if (c.value == Value::WildDrawFour) {
            return "W+4";
        }
        return "W";
    }
    char prefix = '?';
    switch (c.color) {
    case Color::Red:    prefix = 'R'; break;
    case Color::Yellow: prefix = 'Y'; break;
    case Color::Green:  prefix = 'G'; break;
    case Color::Blue:   prefix = 'B'; break;
    case Color::Wild:   prefix = 'W'; break;
    }
    return std::string(1, prefix) + valueLabel(c.value);
}

std::string colorAnsi(Color c) {
    switch (c) {
    case Color::Red:    return "\033[1;31m";
    case Color::Yellow: return "\033[1;33m";
    case Color::Green:  return "\033[1;32m";
    case Color::Blue:   return "\033[1;34m";
    case Color::Wild:   return "\033[1;35m";
    }
    return "\033[0m";
}

} // namespace ichi
