"""Read complete habit titles and descriptions through the actual terminal UI."""

from datetime import date
import json
from pathlib import Path
import tempfile

from habit_view_e2e_test import screen_output


def check_scrolling(path, seed):
    name_parts = [f"word{i:02}" for i in range(80)]
    note_parts = [f"note{i:02}" for i in range(80)]
    habit = seed["habits"][0]
    habit["name"] = "A" * 32 + " I [#red]" + " ".join(name_parts) + "[#end] [#blue]Финиш[#end]"
    habit["description"] = "[#orange]" + " ".join(note_parts) + "[#end] ENDNOTES"
    path.write_text(json.dumps(seed))
    initial = screen_output(path, 80, (b"1", b"\r", b"q"), rows=24)
    assert ("A" * 32 + "I").encode() not in initial, "wrapping joined adjacent words"
    assert b"ENDNOTES" not in initial, "fixture must exceed the visible viewport"
    preview_actions = (b"1",) + (b"\x1b[6~",) * 8 + (b"q",)
    preview = screen_output(path, 80, preview_actions, rows=24)
    for word in name_parts + ["Финиш", "Mon", "Sun", "Less", "More"]:
        assert word.encode() in preview, f"heatmap view hides part of the title or calendar: {word}"
    actions = (b"1", b"\r") + (b"\x1b[6~",) * 12 + (b"q",)
    output = screen_output(path, 80, actions, rows=24)
    for word in name_parts + note_parts + ["Финиш", "ENDNOTES"]:
        assert word.encode() in output, f"scrolling skipped or truncated {word}"
    assert b"[#" not in output, "color tags leaked into the description view"
    end = screen_output(path, 80, (b"1", b"\r", b"\x1b[F", b"q"), rows=24)
    assert b"ENDNOTES" in end, "End did not reveal the end of the description"
    assert json.loads(path.read_text()) == seed, "scrolling modified saved data"


def main():
    with tempfile.TemporaryDirectory(prefix="habit-details-") as directory:
        path = Path(directory) / "data.json"
        seed = {
            "collections": [{"id": 1, "name": "Inbox"}],
            "tasks": [],
            "habits": [{
                "id": 1,
                "name": "I'm [Reading] in [My Room] at [8:00AM] for thirty minutes before breakfast",
                "description": "Read [#orange]carefully[#end] and make notes.",
                "history": {date.today().isoformat(): True},
            }],
        }
        path.write_text(json.dumps(seed))
        for columns in (80, 90):
            for actions in ((b"1", b"q"), (b"1", b"\r", b"q")):
                output = screen_output(path, columns, actions, rows=24)
                assert b"breakfast" in output, "right pane truncates the full habit title"
                assert b"carefully" in output and b"make notes." in output
        assert json.loads(path.read_text()) == seed, "viewing details modified saved data"
        check_scrolling(path, seed)
    print("PASS: full titles at 80/90 columns; long colored text scrolls without changing data")


if __name__ == "__main__":
    main()
