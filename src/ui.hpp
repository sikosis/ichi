#pragma once

#include <string>
#include <vector>

namespace ichi {
namespace ui {

// True when an external prompt tool (gum on Linux/macOS, hum on Haiku)
// is installed and both stdin and stdout are attached to a terminal.
bool available();

// Path/name of the tool in use, or an empty string.
std::string toolName();

// Render a banner. Falls back to the raw text when no tool is present.
std::string style(const std::string& text);

// Yes/No question. Falls back to a plain prompt.
bool confirm(const std::string& prompt, bool defaultYes);

// Single choice from a list. Returns the chosen option verbatim, or an
// empty string if the user aborted. Falls back to a numbered menu.
std::string choose(const std::string& header,
                   const std::vector<std::string>& options,
                   int defaultIndex);

// Free text input. Falls back to std::getline.
std::string input(const std::string& placeholder, const std::string& initial);

// True when the plain-mode input stream hit end-of-file.
bool eof();

} // namespace ui
} // namespace ichi
