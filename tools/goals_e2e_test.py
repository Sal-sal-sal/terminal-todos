"""Exercise Goals through real terminal input and inspect live JSON saves."""

from datetime import date
import fcntl
import json
import os
from pathlib import Path
import signal
import struct
import tempfile
import termios

from habit_autosave_e2e_test import ROOT, seed, send, session
from habit_view_e2e_test import drain


def test_map(path):
    before = seed(path)
    binary = ROOT / "build/term-todos"
    with session(binary, path) as (process, master):
        output = send(master, b"4", b"n", b"Become healthier", b"\r",
                      b"a", b"Improve fitness", b"\r", b"s", b"Plan a workout", b"\r", b" ")
        saved = json.loads(path.read_text())
        assert [(n["title"], n["parent_id"]) for n in saved["goals"]] == [
            ("Become healthier", 0), ("Improve fitness", 1), ("Plan a workout", 2)]
        assert saved["goals"][2]["done"] and not saved["goals"][1]["done"]
        assert saved["habits"] == before["habits"], "Creating Goals changed existing streaks"
        assert b"Goals saved" in output, "Immediate save confirmation is missing"
        output = send(master, b"k", b"H", b"7", b"\r", b" ")
        saved = json.loads(path.read_text())
        assert saved["goals"][-1]["habit_id"] == 7
        assert saved["habits"][0]["history"][date.today().isoformat()]
        for day, done in before["habits"][0]["history"].items():
            assert saved["habits"][0]["history"][day] == done, "Linking reset a streak"
        send(master, b"k", b"k", b"N", "Утренняя прогулка".encode(), b"\r")
        saved = json.loads(path.read_text())
        assert saved["goals"][-1]["parent_id"] == 2
        assert saved["goals"][-1]["habit_id"] == saved["habits"][-1]["id"] == 8
        assert saved["habits"][-1]["name"] == "Утренняя прогулка"
        for key in ("collections", "tasks", "accomplishments"):
            assert saved[key] == before[key], f"Goals changed {key}"
        snapshot = path.read_bytes()
        send(master, b"n", b"\r", b"a", b"cancelled", b"\x1b")
        assert path.read_bytes() == snapshot, "Cancelled or blank creation saved data"
        for width in (80, 90):
            fcntl.ioctl(master, termios.TIOCSWINSZ, struct.pack("HHHH", 24, width, 0, 0))
            os.kill(process.pid, signal.SIGWINCH)
            drain(master, 0.2)
            output = send(master, b"1", b"4")
            assert b"Goals" in output and b"1/2/3/4" in output, "Tabs clipped at narrow width"
        # No clean shutdown: this proves goals are already durable on disk.
        process.kill()
        process.wait(timeout=5)
    with session(binary, path) as (_, master):
        output = send(master, b"4")
        assert b"Become healthier" in output and b"Plan a workout" in output
        assert json.loads(path.read_text()) == saved, "Restart changed goal/history data"
        output = send(master, b"d", b"\r", b"\r", b"4", b"\r")
        assert b"5 goal/task records" in output, "Deletion did not show exact subtree count"
        assert json.loads(path.read_text()) == saved, "Wrong count deleted a goal branch"
        send(master, b"\x1b", b"d", b"\x1b")
        assert json.loads(path.read_text()) == saved, "Cancelled deletion changed data"
        send(master, b"d", b"\r", b"5", b"\r")
        deleted = json.loads(path.read_text())
        assert not deleted.get("goals"), "Confirmed branch deletion was not saved"
        for key in ("habits", "tasks", "collections", "accomplishments"):
            assert deleted[key] == saved[key], f"Branch deletion changed {key}"


def test_save_failure(path):
    before = seed(path)
    blocker = path.parent / ".term-todos.tmp"
    blocker.mkdir()
    with session(ROOT / "build/term-todos", path) as (_, master):
        output = send(master, b"4", b"n", b"Survive failed save", b"\r")
        assert b"Could not save Goals" in output
        assert json.loads(path.read_text()) == before
        blocker.rmdir()
        send(master, b"a", b"Retry save", b"\r")
        assert len(json.loads(path.read_text())["goals"]) == 2


def test_habit_picker(path):
    before = seed(path)
    before["habits"] += [{"id": i, "name": f"Habit {i}", "history": {}}
                         for i in range(8, 21)]
    path.write_text(json.dumps(before))
    with session(ROOT / "build/term-todos", path) as (_, master):
        output = send(master, b"4", b"n", b"Daily actions", b"\r", b"H",
                      b"\x1b[6~", b"\x1b[6~", b"\x1b[B", b"\r", b"D", b"Keep showing up", b"\r")
        saved = json.loads(path.read_text())
        assert saved["goals"][-1]["habit_id"] == 20, "Picker cannot reach habits below first page"
        assert saved["goals"][-1]["description"] == "Keep showing up"
        assert saved["habits"] == before["habits"], "Choosing a habit changed its history"
        assert b"Habit 20" in output and b"Keep showing up" in output
        output = send(master, b"?", b"\x1b")
        assert b"Goals / keyboard guide" in output


def test_scrolling(path):
    before = seed(path)
    before["goals"] = [{"id": i, "parent_id": 0 if i == 1 else 1,
                        "title": f"Step {i}", "done": False} for i in range(1, 46)]
    path.write_text(json.dumps(before))
    with session(ROOT / "build/term-todos", path) as (_, master):
        send(master, b"4", *([b"j"] * 44))
        output = send(master, b"1", b"4")
        assert b"Step 45" in output, "Last goal is not reachable by scrolling"
        assert json.loads(path.read_text()) == before, "Viewing Goals mutated stored data"


if __name__ == "__main__":
    with tempfile.TemporaryDirectory(prefix="goals-e2e-") as directory:
        path = Path(directory) / "data.json"
        test_map(path)
        test_save_failure(path)
        test_habit_picker(path)
        test_scrolling(path)
    print("PASS: Goals terminal creation, nesting, habits, autosave, restart and safe deletion")
