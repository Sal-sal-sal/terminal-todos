#include "backend/models/habit_markup.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void check(bool value, const char* message) {
    if (value) return;
    std::cerr << "FAIL: " << message << '\n';
    std::exit(1);
}

} // namespace

int main() {
    using namespace term_todos;

    const std::string explicit_name =
        "Do [#red]hard[#green]useful[#orange]warm[#blue]calm[#end] work";
    const auto spans = parse_habit_markup(explicit_name, true);
    check(spans.size() == 6, "explicit color changes were not parsed");
    check(spans[0].text == "Do " && spans[0].tone == HabitTone::Plain,
          "ordinary prefix changed");
    check(spans[1].text == "hard" && spans[1].tone == HabitTone::Red,
          "red segment missing");
    check(spans[2].text == "useful" && spans[2].tone == HabitTone::Green,
          "green segment missing");
    check(spans[3].text == "warm" && spans[3].tone == HabitTone::Orange,
          "orange segment missing");
    check(spans[4].text == "calm" && spans[4].tone == HabitTone::Blue,
          "blue segment missing");
    check(spans[5].text == " work" && spans[5].tone == HabitTone::Plain,
          "end tag did not restore ordinary text");
    check(plain_habit_text(explicit_name, true) == "Do hardusefulwarmcalm work",
          "color tags were not hidden");

    const auto legacy = parse_habit_markup("i'm [running] in [mountain] at [6:00]", true);
    check(legacy.size() == 6 && legacy[1].tone == HabitTone::Red
          && legacy[3].tone == HabitTone::Green && legacy[5].tone == HabitTone::Blue,
          "older three-part highlighting changed");
    check(plain_habit_text("Do [something]", false) == "Do [something]",
          "description brackets were interpreted as an old formula");
    check(plain_habit_text("Try [#purple]text", true) == "Try [#purple]text",
          "unknown tag was hidden or treated as legacy markup");
    check(plain_habit_text("Шаг [#orange]сейчас[#end]", false) == "Шаг сейчас",
          "UTF-8 description was damaged");

    std::cout << "habit markup test passed\n";
}
