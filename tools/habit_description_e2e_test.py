"""Exercise habit descriptions, color tags, help, and persistence in a real TUI."""

from datetime import date, timedelta
import json
import os
from pathlib import Path
import subprocess
import tempfile

from habit_formula_preview import render_habit_name
from habit_view_e2e_test import screen_output


ROOT = Path(__file__).resolve().parents[1]
NAME = "Do [#red]R[#end] [#green]G[#end] [#orange]O[#end] [#blue]B[#end]"
DESCRIPTION = (
    "At [#orange]sunrise[#end] in [#green]library[#end], "
    "read for twenty focused minutes before work."
)


def preview(path, color, installed=False):
    env = dict(os.environ, TERM_TODOS_DATA=str(path), TERM_TODO_SPLASH_COLOR=color)
    command = (Path.home() / ".local/bin/term-todo-splash" if installed
               else ROOT / "tools/term-todo-splash")
    return subprocess.check_output([str(command)], env=env).decode()


def main():
    yesterday = (date.today() - timedelta(days=1)).isoformat()
    old_habit = {
        "id": 1,
        "name": "i'm [reading] in [library] at [8:00]",
        "history": {yesterday: True},
    }
    with tempfile.TemporaryDirectory(prefix="habit-description-") as directory:
        path = Path(directory) / "data.json"
        path.write_text(json.dumps({
            "collections": [{"id": 1, "name": "Inbox"}],
            "tasks": [], "habits": [old_habit],
        }))
        output = screen_output(path, 80, (
            b"1", b"n", NAME.encode(), b"\r", b"D", DESCRIPTION.encode(), b"\r", b"q",
        ))
        data = json.loads(path.read_text())
        assert data["habits"][0]["history"] == old_habit["history"]
        assert data["habits"][1]["name"] == NAME
        assert data["habits"][1]["description"] == DESCRIPTION
        assert b"DESCRIPTION" in output and b"sunrise" in output

        plain = preview(path, "never")
        assert "Do R G O B" in plain and "[#" not in plain
        assert "i'm reading in library at 8:00" in plain
        colored = preview(path, "always")
        for code, letter in (("1;91", "R"), ("1;92", "G"),
                             ("1;38;2;255;166;74", "O"), ("1;94", "B")):
            assert f"\033[{code}m{letter}\033[0m" in colored
        assert "[#" not in colored
        assert render_habit_name(NAME, False, 80) == "Do R G O B"
        if os.environ.get("CHECK_INSTALLED_SPLASH") == "1":
            installed = preview(path, "always", installed=True)
            assert "\033[1;38;2;255;166;74mO\033[0m" in installed
            assert "[#" not in installed

        color_screen = screen_output(path, 80, (b"1", b"q"), raw=True, truecolor=True)
        assert b"\x1b[38;2;255;166;74m" in color_screen, "orange was not truecolor"

        restarted = screen_output(path, 80, (b"1", b"j", b"\r", b"?", b"?", b"q"))
        for token in (b"DESCRIPTION", b"sunrise", b"library", b"COLOR TAGS",
                      b"[#red]", b"[#green]", b"[#orange]", b"[#blue]", b"[#end]"):
            assert token in restarted, token
        assert json.loads(path.read_text()) == data, "viewing details modified saved data"
    print("PASS: habit description and all four colors survive restart; ? shows full help")


if __name__ == "__main__":
    main()
