#pragma once

#include <ftxui/dom/elements.hpp>

namespace term_todos::theme {

// A focused, dim palette so the active column/card reads instantly while the
// rest of the UI stays calm.

inline ftxui::Color accent() { return ftxui::Color::Default; } // focus: empty (terminal default)
inline ftxui::Color muted() { return ftxui::Color::GrayLight; }
inline ftxui::Color dim() { return ftxui::Color::GrayDark; }
inline ftxui::Color good() { return ftxui::Color::Green; }  // Done / done cell
inline ftxui::Color warn() { return ftxui::Color::Yellow; } // In Progress
inline ftxui::Color todo() { return ftxui::Color::BlueLight; } // Todo

// GitHub-like green intensity levels for the heatmap (0 = none -> 4 = max).
inline ftxui::Color heatmap_level(int level) {
  switch (level) {
  case 0:
    return ftxui::Color::RGB(34, 37, 46); // empty
  case 1:
    return ftxui::Color::RGB(14, 68, 41); // low
  case 2:
    return ftxui::Color::RGB(0, 109, 50); // mid
  case 3:
    return ftxui::Color::RGB(38, 166, 65); // high
  default:
    return ftxui::Color::RGB(57, 211, 83); // max
  }
}

// Picks a color for a label chip so different labels are visually distinct.
ftxui::Color label_color(const std::string &label);

} // namespace term_todos::theme
