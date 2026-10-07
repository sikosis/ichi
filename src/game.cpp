#include "game.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <thread>

#include "ui.hpp"

namespace ichi {

namespace {

std::string trim(const std::string& s) {
    std::size_t begin = 0;
    std::size_t end = s.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(s[begin]))) {
        ++begin;
    }
    while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }
    return s.substr(begin, end - begin);
}

std::string lower(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

bool parseInt(const std::string& s, int& out) {
    std::istringstream in(s);
    int value = 0;
    in >> value;
    if (in.fail()) {
        return false;
    }
    in >> std::ws;
    if (!in.eof()) {
        return false;
    }
    out = value;
    return true;
}

const char* kReset = "\033[0m";
const char* kBold = "\033[1m";
const char* kDim = "\033[2m";

bool fastMode() {
    static const bool fast = std::getenv("ICHI_FAST") != nullptr;
    return fast;
}

void pace(int millis) {
    if (!fastMode()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(millis));
    }
}

// Pulls the "[token]" out of a menu entry such as "[3] R7".
std::string menuToken(const std::string& entry) {
    std::size_t open = entry.find('[');
    if (open == std::string::npos) {
        return "";
    }
    std::size_t close = entry.find(']', open + 1);
    if (close == std::string::npos) {
        return "";
    }
    return entry.substr(open + 1, close - open - 1);
}

} // namespace

Game::Game(bool americanSpelling) : american_(americanSpelling) {}

void Game::run() {
    std::cout << "\033[3J\033[2J\033[H";
    stats_.load("ichi.stats");

    bool playAgain = true;
    while (playAgain && !quit_) {
        setup();
        turns_ = 0;
        playGame();
        if (quit_) {
            break;
        }
        playAgain = ui::confirm("Play again?", true);
    }

    stats_.save("ichi.stats");
    std::cout << "\n" << stats_.summary();
    std::cout << kDim << "\nThanks for playing ICHI.\n"
              << "Designed by Sikosis\n" << kReset;
}

void Game::playGame() {
    while (!quit_) {
        displayState();
        Player& p = players_[static_cast<std::size_t>(current_)];
        ++turns_;

        if (p.isAI()) {
            aiMove(p);
        } else {
            humanMove(p);
        }

        if (quit_) {
            return;
        }

        if (!p.hasCard()) {
            announceWinner(p);
            stats_.record(!p.isAI(), turns_);
            return;
        }

        current_ = nextIndex(advance_);
    }
}

void Game::announceWinner(const Player& p) const {
    std::cout << "\n" << kBold << (p.isAI() ? colorAnsi(Color::Red) : colorAnsi(Color::Green))
              << "==================================================\n"
              << "  " << p.name() << " wins the game!\n"
              << "==================================================" << kReset << "\n";
    if (!p.isAI()) {
        std::cout << "  " << kBold << "Congratulations, " << p.name()
                  << "! You are the ICHI champion!" << kReset << "\n";
    } else {
        std::cout << kDim << "  Better luck next time.\n" << kReset;
    }
}

std::string Game::colour(bool capitalised) const {
    std::string word = american_ ? "color" : "colour";
    if (capitalised && !word.empty()) {
        word[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(word[0])));
    }
    return word;
}

void Game::setup() {
    std::cout << "\033[3J\033[2J\033[H";
    if (ui::available()) {
        std::cout << "\n" << ui::style("ICHI") << "\n"
                  << kDim << "  the one-card game\n"
                  << "  Designed by Sikosis\n" << kReset;
    } else {
        std::cout << kBold << colorAnsi(Color::Red) << "  I C H I" << kReset
                  << kDim << "  -  the one-card game\n"
                  << "  Designed by Sikosis\n" << kReset;
    }

    std::string name = "Walter";
    std::string answer = trim(ui::input("What's your name? (blank for Walter)", "Walter"));
    if (!answer.empty()) {
        name = answer.substr(0, 24);
    }

    int opponents = 3;
    if (ui::available()) {
        std::string picked = ui::choose("How many computer opponents?",
                                        {"[1] 1 opponent", "[2] 2 opponents", "[3] 3 opponents",
                                         "[q] Quit"},
                                        2);
        std::string token = menuToken(picked);
        if (token == "q") {
            quit_ = true;
            return;
        }
        int parsed = 0;
        if (parseInt(token, parsed)) {
            opponents = std::max(1, std::min(3, parsed));
        }
    } else {
        std::cout << "\nHow many computer opponents? [1-3], 'q' to quit (default 3): ";
        std::string line;
        std::getline(std::cin, line);
        std::string input = lower(trim(line));
        if (input == "q" || input == "quit") {
            quit_ = true;
            return;
        }
        int parsed = 0;
        if (!input.empty() && parseInt(input, parsed)) {
            opponents = std::max(1, std::min(3, parsed));
        }
    }

    static const char* names[] = {"CPU Aoi", "CPU Ren", "CPU Yuki"};
    players_.clear();
    players_.emplace_back(name, false);
    for (int i = 0; i < opponents; ++i) {
        players_.emplace_back(names[i], true);
    }

    deal();
}

void Game::deal() {
    deck_.buildStandard();
    deck_.shuffle();
    discard_.clear();

    for (int round = 0; round < 7; ++round) {
        for (Player& p : players_) {
            p.addCard(drawCard());
        }
    }

    // First card: keep drawing until a plain number card so no opening
    // special effects complicate the first turn.
    Card first = drawCard();
    while (isWild(first) || !isNumber(first)) {
        deck_.add(first);
        Card next = drawCard();
        first = next;
    }
    discard_.push_back(first);
    currentColor_ = first.color;
    direction_ = 1;
    current_ = 0;
}

Card Game::drawCard() {
    if (deck_.empty()) {
        if (discard_.size() <= 1) {
            throw std::runtime_error("no cards left to draw");
        }
        Card top = discard_.back();
        discard_.pop_back();
        deck_.addAll(discard_);
        deck_.shuffle();
        discard_.clear();
        discard_.push_back(top);
    }
    ++stats_.total_draws;
    return deck_.draw();
}

void Game::drawFor(Player& p, int count) {
    for (int i = 0; i < count; ++i) {
        p.addCard(drawCard());
    }
    if (p.count() > 1) {
        p.setSaidIchi(false);
    }
}

const Card& Game::topCard() const {
    return discard_.back();
}

bool Game::canPlay(const Card& c, const Player& p) const {
    if (c.value == Value::Wild) {
        return true;
    }
    if (c.value == Value::WildDrawFour) {
        // Legal only when the hand holds no card of the active color.
        for (const Card& h : p.hand()) {
            if (h.color == currentColor_) {
                return false;
            }
        }
        return true;
    }
    if (c.color == currentColor_) {
        return true;
    }
    if (!isWild(topCard()) && c.value == topCard().value) {
        return true;
    }
    return false;
}

std::vector<int> Game::playableIndices(const Player& p) const {
    std::vector<int> result;
    for (int i = 0; i < p.count(); ++i) {
        if (canPlay(p.hand()[static_cast<std::size_t>(i)], p)) {
            result.push_back(i);
        }
    }
    return result;
}

void Game::displayState() const {
    std::cout << "\n" << kDim << "--------------------------------------------------"
              << kReset << "\n";

    for (std::size_t i = 0; i < players_.size(); ++i) {
        const Player& p = players_[i];
        bool turn = static_cast<int>(i) == current_;
        std::cout << (turn ? kBold : "") << (turn ? "> " : "  ") << p.name() << kReset
                  << " [" << p.count() << " card" << (p.count() == 1 ? "" : "s") << "]";
        if (p.count() == 1 && p.saidIchi()) {
            std::cout << kDim << " (ichi!)" << kReset;
        }
        std::cout << "\n";
    }

    std::cout << "\n  Discard: " << colorAnsi(topCard().color) << "[" << cardLabel(topCard())
              << "]" << kReset
              << "   Active " << colour() << ": " << colorAnsi(currentColor_)
              << colorName(currentColor_)
              << kReset << "\n"
              << "  Deck: " << deck_.size() << " card" << (deck_.size() == 1 ? "" : "s")
              << "   Direction: " << (direction_ == 1 ? "clockwise" : "counter-clockwise")
              << "\n";
}

void Game::displayHand(const Player& p) const {
    std::vector<int> playable = playableIndices(p);
    std::cout << "\n  Your hand:\n";
    for (int i = 0; i < p.count(); ++i) {
        bool ok = std::find(playable.begin(), playable.end(), i) != playable.end();
        std::cout << "   " << (ok ? kBold : kDim) << "[" << i << "] " << kReset
                  << colorAnsi(p.hand()[static_cast<std::size_t>(i)].color)
                  << cardLabel(p.hand()[static_cast<std::size_t>(i)]) << kReset
                  << (ok ? "" : kDim) << (ok ? "" : "  (no match)") << kReset << "\n";
    }
}

void Game::humanMove(Player& p) {
    advance_ = 1;
    displayHand(p);

    while (true) {
        std::string answer = ui::input("Card number  |  d = draw  |  q = quit", "");
        std::string input = lower(trim(answer));

        if (input.empty()) {
            if (ui::eof()) {
                quit_ = true;
                return;
            }
            continue;
        }
        if (input == "q" || input == "quit") {
            quit_ = true;
            return;
        }
        if (input == "d" || input == "draw") {
            drawTurn(p);
            return;
        }

        int index = 0;
        if (!parseInt(input, index) || index < 0 || index >= p.count()) {
            std::cout << kDim << "  Not a valid choice." << kReset << "\n";
            continue;
        }
        if (!canPlay(p.hand()[static_cast<std::size_t>(index)], p)) {
            std::cout << kDim << "  That card doesn't match the " << colour() << " or value."
                      << kReset << "\n";
            continue;
        }

        resolvePlay(p, index);
        return;
    }
}

void Game::drawTurn(Player& p) {
    Card c = drawCard();
    p.addCard(c);
    p.setSaidIchi(false);
    std::cout << "  You drew " << colorAnsi(c.color) << cardLabel(c) << kReset << ".\n";
    if (canPlay(c, p) && ui::confirm("Play the drawn card?", false)) {
        resolvePlay(p, p.count() - 1);
    }
}

void Game::aiMove(Player& p) {
    advance_ = 1;
    pace(500);

    std::vector<int> playable = playableIndices(p);
    if (!playable.empty()) {
        int nextPlayer = nextIndex(1);
        int choice = playable[0];

        // If the next player is close to winning, hit them with an action card.
        if (players_[static_cast<std::size_t>(nextPlayer)].count() <= 2) {
            for (int idx : playable) {
                Value v = p.hand()[static_cast<std::size_t>(idx)].value;
                if (v == Value::DrawTwo || v == Value::WildDrawFour ||
                    v == Value::Skip || v == Value::Reverse) {
                    choice = idx;
                    break;
                }
            }
        } else {
            // Otherwise prefer a plain matching card over a wild.
            for (int idx : playable) {
                if (!isWild(p.hand()[static_cast<std::size_t>(idx)])) {
                    choice = idx;
                    break;
                }
            }
        }

        resolvePlay(p, choice);
        return;
    }

    Card drawn = drawCard();
    p.addCard(drawn);
    p.setSaidIchi(false);
    say(p.name() + " draws a card.");
    if (canPlay(drawn, p)) {
        pace(400);
        resolvePlay(p, p.count() - 1);
    }
}

void Game::resolvePlay(Player& p, int index) {
    Card c;
    if (!p.playCard(static_cast<std::size_t>(index), c)) {
        return;
    }
    discard_.push_back(c);

    if (isWild(c)) {
        currentColor_ = chooseColor(p);
        std::cout << "  " << p.name() << " plays " << colorAnsi(Color::Wild) << cardLabel(c)
                  << kReset << " and chooses " << colorAnsi(currentColor_)
                  << colorName(currentColor_) << kReset << ".\n";
    } else {
        currentColor_ = c.color;
        std::cout << "  " << p.name() << " plays " << colorAnsi(c.color) << cardLabel(c)
                  << kReset << ".\n";
    }

    advance_ = 1;
    switch (c.value) {
    case Value::Skip:
        say(p.name() + " skips the next player!");
        advance_ = 2;
        break;
    case Value::Reverse:
        direction_ = -direction_;
        if (players_.size() == 2) {
            advance_ = 2;
            say("Reverse! " + p.name() + " plays again.");
        } else {
            say("Reverse! Play direction flips.");
        }
        break;
    case Value::DrawTwo: {
        Player& victim = players_[static_cast<std::size_t>(nextIndex(1))];
        drawFor(victim, 2);
        say(victim.name() + " draws 2 and is skipped.");
        advance_ = 2;
        break;
    }
    case Value::WildDrawFour: {
        Player& victim = players_[static_cast<std::size_t>(nextIndex(1))];
        drawFor(victim, 4);
        say(victim.name() + " draws 4 and is skipped.");
        advance_ = 2;
        break;
    }
    default:
        break;
    }

    if (p.count() == 1) {
        handleIchi(p);
    }
}

void Game::handleIchi(Player& p) {
    if (p.isAI()) {
        pace(300);
        if (randomInt(0, 99) < 70) {
            p.setSaidIchi(true);
            stats_.noteIchiCall();
            say(p.name() + " shouts \"ICHI!\"");
        } else {
            std::cout << "  " << kBold << p.name() << " forgot to call ichi!" << kReset << "\n";
            stats_.noteIchiPenalty();
            drawFor(p, 2);
            say(p.name() + " draws 2 as a penalty.");
        }
        return;
    }

    bool called = ui::available() ? ui::confirm("One card left - call ICHI!?", true)
                                  : promptIchi();
    if (called) {
        p.setSaidIchi(true);
        stats_.noteIchiCall();
        say("You shout \"ICHI!\"");
    } else {
        std::cout << "  " << kBold << "You forgot to call ichi!" << kReset << "\n";
        stats_.noteIchiPenalty();
        drawFor(p, 2);
        say("You draw 2 as a penalty.");
    }
}

bool Game::promptIchi() {
    std::cout << "\n  " << kBold << "One card left! Type 'ichi' to call it: " << kReset;
    std::string line;
    if (!std::getline(std::cin, line)) {
        return false;
    }
    return lower(trim(line)) == "ichi";
}

Color Game::chooseColor(Player& p) {
    if (p.isAI()) {
        Color chosen = bestColorFor(p);
        return chosen;
    }

    if (ui::available()) {
        std::string answer = lower(trim(ui::input("Choose a " + colour() + " (r/y/g/b)", "")));
        if (answer == "y") return Color::Yellow;
        if (answer == "g") return Color::Green;
        if (answer == "b") return Color::Blue;
        if (answer == "r") return Color::Red;
        return Color::Red;
    }

    while (true) {
        std::cout << "  Choose a " << colour() << " - (r)ed, (y)ellow, (g)reen, (b)lue: ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            return Color::Red;
        }
        std::string input = lower(trim(line));
        if (input == "r" || input == "red") return Color::Red;
        if (input == "y" || input == "yellow") return Color::Yellow;
        if (input == "g" || input == "green") return Color::Green;
        if (input == "b" || input == "blue") return Color::Blue;
        std::cout << kDim << "  Please choose r, y, g, or b." << kReset << "\n";
    }
}

Color Game::bestColorFor(const Player& p) const {
    int counts[4] = {0, 0, 0, 0};
    const Color colors[4] = {Color::Red, Color::Yellow, Color::Green, Color::Blue};
    for (const Card& c : p.hand()) {
        for (int i = 0; i < 4; ++i) {
            if (c.color == colors[i]) {
                ++counts[i];
            }
        }
    }
    int best = 0;
    for (int i = 1; i < 4; ++i) {
        if (counts[i] > counts[best]) {
            best = i;
        }
    }
    return colors[best];
}

int Game::nextIndex(int steps) const {
    int n = static_cast<int>(players_.size());
    int idx = (current_ + direction_ * steps) % n;
    if (idx < 0) {
        idx += n;
    }
    return idx;
}

int Game::randomInt(int low, int high) const {
    static std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(low, high);
    return dist(rng);
}

void Game::say(const std::string& msg) const {
    std::cout << kDim << "  " << msg << kReset << "\n";
}

} // namespace ichi
