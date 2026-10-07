#include "ui.hpp"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace ichi {
namespace ui {

namespace {

bool eofSeen = false;

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

bool envDisablesUi() {
    const char* value = std::getenv("ICHI_UI");
    if (value == nullptr) {
        return false;
    }
    std::string s(value);
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s == "plain" || s == "none" || s == "off" || s == "0";
}

std::string findInPath(const std::string& name) {
    const char* path = std::getenv("PATH");
    if (path == nullptr) {
        return "";
    }
    std::string joined(path);
    std::size_t start = 0;
    while (start <= joined.size()) {
        std::size_t end = joined.find(':', start);
        std::string dir = joined.substr(
            start, end == std::string::npos ? std::string::npos : end - start);
        if (dir.empty()) {
            dir = ".";
        }
        std::string full = dir + "/" + name;
        if (access(full.c_str(), X_OK) == 0) {
            return full;
        }
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    return "";
}

std::string detectTool() {
    if (envDisablesUi()) {
        return "";
    }
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
        return "";
    }
#ifdef __HAIKU__
    const char* order[] = {"hum", "gum"};
#else
    const char* order[] = {"gum", "hum"};
#endif
    for (const char* name : order) {
        std::string found = findInPath(name);
        if (!found.empty()) {
            return found;
        }
    }
    return "";
}

const std::string& tool() {
    static const std::string path = detectTool();
    return path;
}

// Run a program, capturing stdout. Stderr is left attached to the terminal
// so the interactive prompt stays visible. Returns the exit status.
int runCapture(const std::vector<std::string>& args, std::string& out) {
    out.clear();
    int pipefd[2];
    if (pipe(pipefd) != 0) {
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (pid == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        std::vector<char*> argv;
        argv.reserve(args.size() + 1);
        for (const std::string& a : args) {
            argv.push_back(const_cast<char*>(a.c_str()));
        }
        argv.push_back(nullptr);
        execvp(argv[0], argv.data());
        _exit(127);
    }

    close(pipefd[1]);
    char buffer[4096];
    ssize_t n = 0;
    while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
        out.append(buffer, static_cast<std::size_t>(n));
    }
    close(pipefd[0]);

    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt << std::flush;
    std::string line;
    if (!std::getline(std::cin, line)) {
        eofSeen = true;
        return "";
    }
    eofSeen = false;
    return line;
}

} // namespace

bool available() {
    return !tool().empty();
}

bool eof() {
    return eofSeen;
}

std::string toolName() {
    return tool();
}

std::string style(const std::string& text) {
    if (!available()) {
        return text;
    }
    std::string out;
    int code = runCapture({tool(), "style", "--border", "rounded", "--padding", "0 3",
                           "--foreground", "212", "--bold", text},
                          out);
    if (code != 0 || out.empty()) {
        return text;
    }
    return out;
}

bool confirm(const std::string& prompt, bool defaultYes) {
    if (available()) {
        std::string ignored;
        std::string flag = defaultYes ? "--default=yes" : "--default=no";
        int code = runCapture({tool(), "confirm", flag, prompt}, ignored);
        return code == 0;
    }

    std::string line = trim(readLine(prompt + (defaultYes ? " [Y/n] " : " [y/N] ")));
    if (eof()) {
        return false;
    }
    if (line.empty()) {
        return defaultYes;
    }
    for (char& c : line) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return line == "y" || line == "yes";
}

std::string choose(const std::string& header,
                   const std::vector<std::string>& options,
                   int defaultIndex) {
    if (options.empty()) {
        return "";
    }
    if (defaultIndex < 0 || defaultIndex >= static_cast<int>(options.size())) {
        defaultIndex = 0;
    }

    if (available()) {
        std::vector<std::string> args = {tool(), "choose", "--header", header,
                                         "--cursor", "> "};
        args.insert(args.end(), options.begin(), options.end());
        std::string out;
        int code = runCapture(args, out);
        if (code == 0) {
            std::string selected = trim(out);
            for (const std::string& option : options) {
                if (option == selected) {
                    return option;
                }
            }
        }
        return "";
    }

    std::cout << "  " << header << "\n";
    for (std::size_t i = 0; i < options.size(); ++i) {
        std::cout << "    " << (i + 1) << ") " << options[i] << "\n";
    }
    std::string line = trim(readLine("  Select [1-" + std::to_string(options.size()) +
                                     "] (default " + std::to_string(defaultIndex + 1) + "): "));
    if (eof() || line.empty()) {
        return options[static_cast<std::size_t>(defaultIndex)];
    }
    std::istringstream in(line);
    int value = 0;
    in >> value;
    if (in.fail() || value < 1 || value > static_cast<int>(options.size())) {
        return options[static_cast<std::size_t>(defaultIndex)];
    }
    return options[static_cast<std::size_t>(value - 1)];
}

std::string input(const std::string& placeholder, const std::string& initial) {
    if (available()) {
        std::string out;
        int code = runCapture({tool(), "input", "--placeholder", placeholder, "--value", initial}, out);
        if (code == 0) {
            std::string value = out;
            while (!value.empty() && (value.back() == '\n' || value.back() == '\r')) {
                value.pop_back();
            }
            return value;
        }
        return initial;
    }
    std::string value = readLine(placeholder + ": ");
    if (eof()) {
        return "";
    }
    return value;
}

} // namespace ui
} // namespace ichi
