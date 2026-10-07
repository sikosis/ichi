#pragma once

#include <string>

namespace ichi {

struct Stats {
    long long games = 0;
    long long wins = 0;
    long long losses = 0;
    long long current_streak = 0;   // positive: win streak, negative: loss streak
    long long best_win_streak = 0;
    long long total_turns = 0;
    long long total_draws = 0;
    long long ichi_calls = 0;
    long long ichi_penalties = 0;
    std::string last_played;
    std::string last_result;

    void record(bool win, long long turns);
    void noteDraws(long long n = 1);
    void noteIchiCall();
    void noteIchiPenalty();

    bool load(const std::string& path);
    bool save(const std::string& path) const;
    std::string summary() const;
};

} // namespace ichi
