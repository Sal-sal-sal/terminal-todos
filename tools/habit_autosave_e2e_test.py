"""Create habits in a real terminal and check persistence before exiting."""

import argparse
from contextlib import contextmanager
from datetime import date, timedelta
import fcntl
import json
import os
from pathlib import Path
import pty
import struct
import subprocess
import tempfile
import termios
import time

from habit_view_e2e_test import drain


ROOT = Path(__file__).resolve().parents[1]
NAME = "Read quantum physics"


@contextmanager
def session(binary, path):
    master, slave = pty.openpty()
    fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", 28, 100, 0, 0))
    process = subprocess.Popen(
        [str(binary), str(path)], stdin=slave, stdout=slave, stderr=slave,
        env=dict(os.environ, TERM="xterm-256color"),
    )
    os.close(slave)
    try:
        # Wait for the first rendered footer, not a fixed startup delay.
        # FTXUI installs raw mode before drawing; earlier input can be flushed.
        ready = b""
        deadline = time.monotonic() + 5
        while b"quit" not in ready and time.monotonic() < deadline and process.poll() is None:
            ready += drain(master, 0.1)
        assert process.poll() is None, "TUI did not start"
        assert b"quit" in ready, "TUI did not render its first frame"
        yield process, master
    finally:
        if process.poll() is None:
            process.kill()
        process.wait(timeout=5)
        os.close(master)


def send(master, *keys):
    output = b""
    for key in keys:
        os.write(master, key)
        # A standalone Escape is delivered after the input parser's timeout.
        output += drain(master, 0.35 if key == b"\x1b" else 0.2)
    return output


def seed(path):
    yesterday = (date.today() - timedelta(days=1)).isoformat()
    payload = {
        "schema_version": 6,
        "collections": [{"id": 1, "name": "Inbox"}],
        "tasks": [],
        "accomplishments": [{"title": "Keep this task", "done": False}],
        "habits": [{"id": 7, "name": "Existing habit", "history": {yesterday: True}}],
    }
    path.write_text(json.dumps(payload))
    return payload


def test_creation(binary, path):
    before = seed(path)
    with session(binary, path) as (process, master):
        output = send(master, b"1", b"n", NAME.encode(), b"\r")
        assert process.poll() is None, "Creating a habit exited the app"
        saved = json.loads(path.read_text())
        assert any(habit["name"] == NAME for habit in saved["habits"]), \
            "New habit is not on disk while the app is running"
        assert saved["habits"][0] == before["habits"][0], "Existing history changed"
        for key in ("collections", "tasks", "accomplishments"):
            assert saved[key] == before[key], f"Autosave changed {key}"
        assert b"Habit created and saved" in output, "Save confirmation is missing"
        snapshot = path.read_bytes()
        send(master, b"n", b"\r", b"n", b"cancelled", b"\x1b")
        assert path.read_bytes() == snapshot, "Empty/cancelled creation wrote data"
        send(master, b"n", "Вторая привычка".encode(), b"\r")
        saved = json.loads(path.read_text())
        assert [habit["id"] for habit in saved["habits"]] == [7, 8, 9]
        assert saved["habits"][-1]["name"] == "Вторая привычка"
        # Simulate a terminal crash without running the app's exit-time save.
        process.kill()
        process.wait(timeout=5)
    with session(binary, path) as (_, master):
        output = send(master, b"1")
        assert NAME.encode() in output, "Saved habit disappeared after restart"
        assert json.loads(path.read_text()) == saved, "Restart changed stored data"


def test_save_failure(binary, path):
    before = seed(path)
    blocker = path.parent / ".term-todos.tmp"
    blocker.mkdir()
    with session(binary, path) as (process, master):
        output = send(master, b"1", b"n", NAME.encode(), b"\r")
        assert b"Could not save habit" in output, "Save error was not shown"
        assert b"Habit created and saved" not in output, "Failed save reported success"
        assert json.loads(path.read_text()) == before, "Failed save damaged old data"
        blocker.rmdir()
        send(master, b"q")
        process.wait(timeout=5)
        assert process.returncode == 0, "Exit-time retry failed"
        names = [habit["name"] for habit in json.loads(path.read_text())["habits"]]
        assert names == ["Existing habit", NAME], "Failed save lost the pending habit"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, default=ROOT / "build/term-todos")
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="habit-autosave-") as directory:
        path = Path(directory) / "data.json"
        test_creation(args.binary, path)
        test_save_failure(args.binary, path)
    print("PASS: habits save immediately, survive crashes, and report/recover from save errors")


if __name__ == "__main__":
    main()
