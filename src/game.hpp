#pragma once

#include <string>
#include <vector>

#include "card.hpp"
#include "deck.hpp"
#include "player.hpp"
#include "stats.hpp"

namespace ichi {

class Game {
public:
    explicit Game(bool americanSpelling = false);
    void run();

private:
    Deck deck_;
    std::vector<Card> discard_;
    std::vector<Player> players_;
    Color currentColor_ = Color::Red;
    int direction_ = 1;
    int current_ = 0;
    int advance_ = 1;
    bool quit_ = false;
    bool american_ = false;
    Stats stats_;
    long long turns_ = 0;

    void setup();
    void deal();
    void playGame();
    void announceWinner(const Player& p) const;
    Card drawCard();
    void drawFor(Player& p, int count);
    const Card& topCard() const;

    bool canPlay(const Card& c, const Player& p) const;
    std::vector<int> playableIndices(const Player& p) const;

    void displayState() const;
    void displayHand(const Player& p) const;

    void humanMove(Player& p);
    void drawTurn(Player& p);
    void aiMove(Player& p);

    void resolvePlay(Player& p, int index);
    Color chooseColor(Player& p);
    Color bestColorFor(const Player& p) const;

    void handleIchi(Player& p);
    bool promptIchi();

    int nextIndex(int steps) const;
    int randomInt(int low, int high) const;

    std::string colour(bool capitalised = false) const;
    void say(const std::string& msg) const;
};

} // namespace ichi
