#include "stats.hpp"

#include <cctype>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

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

std::string nowStamp() {
    std::time_t t = std::time(nullptr);
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &t);
#else
    localtime_r(&t, &local);
#endif
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &local);
    return buffer;
}

std::string ratio(long long wins, long long losses) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(2);
    if (losses == 0) {
        if (wins == 0) {
            out << "0.00";
        } else {
            out << "inf";
        }
    } else {
        out << static_cast<double>(wins) / static_cast<double>(losses);
    }
    return out.str();
}

} // namespace

void Stats::record(bool win, long long turns) {
    ++games;
    total_turns += turns;
    if (win) {
        ++wins;
        current_streak = current_streak > 0 ? current_streak + 1 : 1;
        if (current_streak > best_win_streak) {
            best_win_streak = current_streak;
        }
        last_result = "win";
    } else {
        ++losses;
        current_streak = current_streak < 0 ? current_streak - 1 : -1;
        last_result = "loss";
    }
    last_played = nowStamp();
}

void Stats::noteDraws(long long n) {
    total_draws += n;
}

void Stats::noteIchiCall() {
    ++ichi_calls;
}

void Stats::noteIchiPenalty() {
    ++ichi_penalties;
}

bool Stats::load(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        return false;
    }

    std::string line;
    while (std::getline(in, line)) {
        std::size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = trim(line.substr(0, eq));
        std::string value = trim(line.substr(eq + 1));

        long long number = 0;
        bool numeric = true;
        try {
            std::size_t used = 0;
            number = std::stoll(value, &used);
            if (used != value.size()) {
                numeric = false;
            }
        } catch (...) {
            numeric = false;
        }

        if (key == "last_played") {
            last_played = value;
        } else if (key == "last_result") {
            last_result = value;
        } else if (numeric) {
            if (key == "games") games = number;
            else if (key == "wins") wins = number;
            else if (key == "losses") losses = number;
            else if (key == "current_streak") current_streak = number;
            else if (key == "best_win_streak") best_win_streak = number;
            else if (key == "total_turns") total_turns = number;
            else if (key == "total_draws") total_draws = number;
            else if (key == "ichi_calls") ichi_calls = number;
            else if (key == "ichi_penalties") ichi_penalties = number;
        }
    }
    return true;
}

bool Stats::save(const std::string& path) const {
    std::ofstream out(path, std::ios::trunc);
    if (!out) {
        return false;
    }
    out << "games=" << games << "\n"
        << "wins=" << wins << "\n"
        << "losses=" << losses << "\n"
        << "current_streak=" << current_streak << "\n"
        << "best_win_streak=" << best_win_streak << "\n"
        << "total_turns=" << total_turns << "\n"
        << "total_draws=" << total_draws << "\n"
        << "ichi_calls=" << ichi_calls << "\n"
        << "ichi_penalties=" << ichi_penalties << "\n"
        << "last_played=" << last_played << "\n"
        << "last_result=" << last_result << "\n";
    return static_cast<bool>(out);
}

std::string Stats::summary() const {
    std::ostringstream out;
    double winRate = games > 0 ? 100.0 * static_cast<double>(wins) / static_cast<double>(games) : 0.0;
    double avgTurns = games > 0 ? static_cast<double>(total_turns) / static_cast<double>(games) : 0.0;

    out << "\033[1m  ICHI STATS\033[0m\n"
        << "    Games played     : " << games << "\n"
        << "    Wins / Losses    : " << wins << " / " << losses << "\n"
        << "    Win:loss ratio   : " << ratio(wins, losses) << "\n"
        << "    Win rate         : " << std::fixed << std::setprecision(1) << winRate << "%\n"
        << "    Current streak   : " << current_streak << "\n"
        << "    Best win streak  : " << best_win_streak << "\n"
        << "    Avg turns / game : " << std::setprecision(1) << avgTurns << "\n"
        << "    Cards drawn      : " << total_draws << "\n"
        << "    Ichi calls       : " << ichi_calls << "\n"
        << "    Ichi penalties   : " << ichi_penalties << "\n"
        << "    Last played      : " << (last_played.empty() ? "never" : last_played);
    if (!last_result.empty()) {
        out << " (" << last_result << ")";
    }
    out << "\n";
    return out.str();
}

} // namespace ichi
