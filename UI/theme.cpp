#include "UI/theme.hpp"

#include <string>

namespace term_todos::theme {

namespace {

// Deterministic palette cycled by label hash so each label keeps its color
// across frames.
ftxui::Color palette[] = {
    ftxui::Color::Magenta,
    ftxui::Color::BlueLight,
    ftxui::Color::GreenLight,
    ftxui::Color::Yellow,
    ftxui::Color::Cyan,
    ftxui::Color::RedLight,
};

} // namespace

ftxui::Color label_color(const std::string& label) {
    unsigned h = 5381;
    for (char c : label) h = ((h << 5) + h) + static_cast<unsigned char>(c);
    return palette[h % (sizeof(palette) / sizeof(palette[0]))];
}

} // namespace term_todos::theme
