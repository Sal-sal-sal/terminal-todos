#include "UI/habits/styled_text.hpp"

#include "UI/theme.hpp"
#include "backend/models/habit_markup.hpp"

#include <ftxui/screen/string.hpp>

#include <algorithm>
#include <vector>

namespace term_todos {

namespace {

using namespace ftxui;

struct Glyph {
  std::string text;
  HabitTone tone;
  int cells;
};

Color tone_color(HabitTone tone, bool focused) {
  switch (tone) {
  case HabitTone::Red:
    return Color::RedLight;
  case HabitTone::Green:
    return Color::GreenLight;
  case HabitTone::Orange:
    return Color::RGB(255, 166, 74);
  case HabitTone::Blue:
    return Color::BlueLight;
  case HabitTone::Plain:
    return focused ? theme::accent() : theme::muted();
  }
  return theme::muted();
}

Element colored_piece(const std::string &value, HabitTone tone, bool focused) {
  auto piece = text(value) | color(tone_color(tone, focused));
  if (focused || tone != HabitTone::Plain)
    piece |= bold;
  return piece;
}

Element render_line(const std::vector<Glyph> &glyphs, bool focused) {
  Elements pieces;
  HabitTone previous = HabitTone::Plain;
  std::string run;
  for (const auto &glyph : glyphs) {
    if (!run.empty() && previous != glyph.tone) {
      pieces.push_back(colored_piece(run, previous, focused));
      run.clear();
    }
    previous = glyph.tone;
    run += glyph.text;
  }
  if (!run.empty())
    pieces.push_back(colored_piece(run, previous, focused));
  return pieces.empty() ? text("") : hbox(pieces);
}

} // namespace

ftxui::Element render_habit_text(const std::string &value, int width,
                                 int max_lines, bool focused,
                                 bool legacy_brackets) {
  width = std::max(1, width);
  max_lines = std::max(1, max_lines);
  std::vector<Glyph> glyphs;
  for (const auto &span : parse_habit_markup(value, legacy_brackets)) {
    for (const auto &glyph : ftxui::Utf8ToGlyphs(span.text)) {
      if (!glyph.empty())
        glyphs.push_back({glyph, span.tone, ftxui::string_width(glyph)});
    }
  }

  std::vector<std::vector<Glyph>> lines(1);
  int used = 0;
  bool clipped = false;
  for (std::size_t at = 0; at < glyphs.size();) {
    if (glyphs[at].text == " ") {
      if (used > 0 && used + 1 <= width) {
        lines.back().push_back(glyphs[at]);
        ++used;
      }
      ++at;
      continue;
    }

    std::size_t end = at;
    int word_width = 0;
    while (end < glyphs.size() && glyphs[end].text != " ") {
      word_width += glyphs[end].cells;
      ++end;
    }
    if (used > 0 && used + word_width > width) {
      while (!lines.back().empty() && lines.back().back().text == " ") {
        used -= lines.back().back().cells;
        lines.back().pop_back();
      }
      if (static_cast<int>(lines.size()) == max_lines) {
        clipped = true;
        break;
      }
      lines.emplace_back();
      used = 0;
    }
    while (at < end) {
      const auto &glyph = glyphs[at++];
      if (used + glyph.cells > width) {
        if (static_cast<int>(lines.size()) == max_lines) {
          clipped = true;
          break;
        }
        lines.emplace_back();
        used = 0;
      }
      lines.back().push_back(glyph);
      used += glyph.cells;
    }
    if (clipped)
      break;
  }
  if (clipped) {
    auto &last = lines.back();
    while (!last.empty() && used + 1 > width) {
      used -= last.back().cells;
      last.pop_back();
    }
    last.push_back({"…", HabitTone::Plain, 1});
  }
  ftxui::Elements rows;
  for (const auto &line : lines)
    rows.push_back(render_line(line, focused));
  return ftxui::vbox(rows);
}

} // namespace term_todos
