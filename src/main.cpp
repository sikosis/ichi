#include <exception>
#include <iostream>
#include <string>

#include "game.hpp"

namespace {

void usage(const char* program) {
    std::cout << "Usage: " << program << " [options]\n"
              << "\n"
              << "Options:\n"
              << "  --us     Use American spelling (color) instead of Australian (colour)\n"
              << "  --au     Use Australian spelling (colour) [default]\n"
              << "  -h, --help  Show this help and exit\n"
              << "\n"
              << "Designed by Sikosis\n";
}

} // namespace

int main(int argc, char** argv) {
    bool american = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--us") {
            american = true;
        } else if (arg == "--au" || arg == "--uk") {
            american = false;
        } else if (arg == "-h" || arg == "--help") {
            usage(argv[0]);
            return 0;
        } else {
            std::cerr << "ichi: unknown option '" << arg << "'\n";
            usage(argv[0]);
            return 2;
        }
    }

    try {
        ichi::Game game(american);
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "ichi: " << e.what() << "\n";
        return 1;
    }
    return 0;
}